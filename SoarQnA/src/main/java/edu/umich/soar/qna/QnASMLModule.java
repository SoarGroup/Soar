package edu.umich.soar.qna;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.CountDownLatch;

import sml.Agent;
import sml.Identifier;
import sml.IntElement;
import sml.Kernel;
import sml.WMElement;
import sml.smlRunEventId;
import sml.smlSystemEventId;
import sml.Agent.OutputEventInterface;
import sml.Agent.RunEventInterface;
import sml.Kernel.SystemEventInterface;

public class QnASMLModule {
	
	private final Identifier inputLink;
	private Identifier qnaRegistry;
	
	private final DataSourceManager man;
	
	private long queryCounter;
	private final Map<Long, ResultState> intermediateResults = new HashMap<Long, ResultState>();
	private class ResultState {
		public final Identifier oldParent;
		public final QueryState queryState;
		
		ResultState(Identifier oldParent, QueryState queryState) {
			this.oldParent = oldParent;
			this.queryState = queryState;
		}
	};
	
	private CountDownLatch doneSignal;
	
	private static final String QUERY_COMMAND_NAME = "qna-query";
	private static final String NEXT_COMMAND_NAME = "qna-next";
	
	private static final String NEXT_NAME = "next";
	private static final String RESULT_NAME = "result";
	private static final String QUERY_NAME = "query";
	private static final String SOURCE_NAME = "source";
	private static final String RESULTS_NAME = "results";
	private static final String INCREMENTAL_NAME = "incremental";
	private static final String PARAMETERS_NAME = "parameters";
	private static final String NIL_NAME = "nil";
	private static final String FEATURES_NAME = "features";
	private static final String PENDING_NAME = "pending";
	private static final String ID_NAME = "id";
	
	public QnASMLModule(Kernel kernel, Agent agent, DataSourceManager man, CountDownLatch doneSignal) {
		this.inputLink = agent.GetInputLink();
		
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_BEFORE_SHUTDOWN, shutdownHandler, null);
		
		agent.RegisterForRunEvent(smlRunEventId.smlEVENT_AFTER_HALTED, haltHandler, null);
		
		agent.AddOutputHandler(QUERY_COMMAND_NAME, queryCommandHandler, null);
		agent.AddOutputHandler(NEXT_COMMAND_NAME, nextCommandHandler, null);
		agent.SetOutputLinkChangeTracking(true);
		
		this.man = man;
		qnaRegistry = inputLink.CreateIdWME("qna-registry");
		for (Entry<String, Map<String, String>> s : man.getRegistry().entrySet()) {
			Identifier dataSource = qnaRegistry.CreateIdWME(s.getKey());
			
			for (Entry<String, String> q : s.getValue().entrySet()) {
				dataSource.CreateStringWME(QUERY_NAME, q.getKey());
			}
		}
		
		this.doneSignal = doneSignal;
		
		this.queryCounter = 1;
	}
	
	public void close() {
		qnaRegistry.DestroyWME();
	}
	
	private void addFeatures(Identifier features, QueryState queryState) {
		Map<String, List<Object>> row = queryState.next();
		
		for (Entry<String, List<Object>> c : row.entrySet()) {
			for (Object v : c.getValue()) {
				if (v instanceof Long) {
					features.CreateIntWME(c.getKey(), ((Long) v).longValue());
				} else if (v instanceof Integer) {
					features.CreateIntWME(c.getKey(), ((Integer) v).longValue());
				} else if (v instanceof Double) {
					features.CreateFloatWME(c.getKey(), ((Double) v).doubleValue());
				} else {
					features.CreateStringWME(c.getKey(), v.toString());
				}
			}
		}
	}
	
	private void addResult(Long queryId, boolean first, boolean incremental) {
		if (!intermediateResults.containsKey(queryId)) {
			return;
		}
		
		ResultState rs = intermediateResults.get(queryId);
		
		if (incremental) {
			{
				WMElement oldChild = rs.oldParent.FindByAttribute(NEXT_NAME, 0);
				if (oldChild!=null && oldChild.ConvertToStringElement()!=null && oldChild.ConvertToStringElement().GetValue().compareTo(PENDING_NAME)==0) {
					oldChild.DestroyWME();
				}
			}
			
			Identifier newParent = rs.oldParent.CreateIdWME(first?RESULT_NAME:NEXT_NAME);
			addFeatures(newParent.CreateIdWME(FEATURES_NAME), rs.queryState);
			if (rs.queryState.hasNext()) {
				newParent.CreateStringWME(NEXT_NAME, PENDING_NAME);
				intermediateResults.put(queryId, new ResultState(newParent, rs.queryState));				
			} else {
				newParent.CreateStringWME(NEXT_NAME, NIL_NAME);
				intermediateResults.remove(queryId);
			}
		} else {
			Identifier oldParent = rs.oldParent;
			Identifier newParent = null;
			boolean firstRound = first;
			
			do {				
				if (firstRound) {
					newParent = oldParent.CreateIdWME(RESULT_NAME);
					firstRound = false;
				} else {
					newParent = oldParent.CreateIdWME(NEXT_NAME);
				}				
				addFeatures(newParent.CreateIdWME(FEATURES_NAME), rs.queryState);
				
				if (rs.queryState.hasNext()) {
					oldParent = newParent;
				} else {
					newParent.CreateStringWME(NEXT_NAME, NIL_NAME);
				}
			} while (rs.queryState.hasNext());
			
			intermediateResults.remove(queryId);
		}
	}
	
	private final SystemEventInterface shutdownHandler = new SystemEventInterface() {

		@Override
		public void systemEventHandler(int eventID, Object data, Kernel kernel) {
			doneSignal.countDown();
		}
		
	};

	private final RunEventInterface haltHandler = new RunEventInterface() {

		@Override
		public void runEventHandler(int eventID, Object data, Agent agent, int phase) {
			doneSignal.countDown();
		}

	};
	
	private final OutputEventInterface nextCommandHandler = new OutputEventInterface() {

		@Override
		public void outputEventHandler(Object data, String agentName,
				String attributeName, WMElement pWmeAdded) {
			if (pWmeAdded.IsJustAdded() && pWmeAdded.IsIdentifier()) {
				boolean goodNext = false;
				
				Identifier pIdAdded = pWmeAdded.ConvertToIdentifier();
				
				WMElement queryWme = pIdAdded.FindByAttribute(QUERY_NAME, 0);
				IntElement queryInt = queryWme.ConvertToIntElement();
				
				if (queryInt!=null) {
					if (intermediateResults.containsKey(queryInt.GetValue())) {
						addResult(queryInt.GetValue(), false, true);
						goodNext = true;
					}
				}
				
				if (goodNext) {
					pIdAdded.AddStatusComplete();
				} else {
					pIdAdded.AddStatusError();
				}
				
			}
		}
	
	};
	
	private final OutputEventInterface queryCommandHandler = new OutputEventInterface() {

		@Override
		public void outputEventHandler(Object data, String agentName,
				String attributeName, WMElement pWmeAdded) {
			if (pWmeAdded.IsJustAdded() && pWmeAdded.IsIdentifier()) {
				Identifier pIdAdded = pWmeAdded.ConvertToIdentifier();
				boolean goodQuery = true;
				
				String uid = pIdAdded.GetParameterValue(SOURCE_NAME);
				if (uid==null) {
					goodQuery = false;
				}
				
				String queryName = pIdAdded.GetParameterValue(QUERY_NAME);
				if (queryName==null) {
					goodQuery = false;
				}
				
				String results = pIdAdded.GetParameterValue(RESULTS_NAME);
				if (results==null) {
					goodQuery = false;
				}
				
				WMElement paramsWme = pIdAdded.FindByAttribute(PARAMETERS_NAME, 0);
				
				// parse parameters, try executing query
				if (goodQuery && (paramsWme != null) && paramsWme.IsIdentifier()) {
					Identifier paramsId = paramsWme.ConvertToIdentifier();
					Map<Object, List<Object>> queryParams = new HashMap<Object, List<Object>>();
					
					for (int i=0; i<paramsId.GetNumberChildren(); i++) {
						WMElement childWme = paramsId.GetChild(i);
						
						if (!childWme.IsIdentifier()) {
							Object childAttr = null;
							try {
								childAttr = new Long(Long.parseLong(childWme.GetAttribute()));
							} catch (Exception e) {
							}
							if (childAttr == null) {
								try {
									childAttr = new Double(Double.parseDouble(childWme.GetAttribute()));
								} catch (Exception e) {
								}
							}
							if (childAttr == null) {
								childAttr = childWme.GetAttribute();
							}
							
							if (!queryParams.containsKey(childAttr)) {
								queryParams.put(childAttr, new LinkedList<Object>());
							}
							
							if (childWme.ConvertToIntElement()!=null) {
								queryParams.get(childAttr).add(new Long(childWme.ConvertToIntElement().GetValue()));
							} else if (childWme.ConvertToFloatElement()!=null) {
								queryParams.get(childAttr).add(new Double(childWme.ConvertToFloatElement().GetValue()));
							} else {
								queryParams.get(childAttr).add(new String(childWme.ConvertToStringElement().GetValue()));
							}
						}
					}
					
					QueryState queryState = null;
					try {
						queryState = man.executeQuery(uid, queryName, queryParams);
					} catch (Exception e) {
						e.printStackTrace();
						goodQuery = false;
					} 
					
					if ((queryState != null)) {
						pIdAdded.CreateIntWME(ID_NAME, queryCounter);						
						intermediateResults.put(queryCounter, new ResultState(pIdAdded, queryState));
						
						addResult(queryCounter, true, results.compareTo(INCREMENTAL_NAME)==0);
						
						queryCounter++;
					} else {
						goodQuery = false;
					}
				}
				
				if (goodQuery) {
					pIdAdded.AddStatusComplete();
				} else {
					pIdAdded.AddStatusError();
				}
			}			
		}		
	};
	
	
}
