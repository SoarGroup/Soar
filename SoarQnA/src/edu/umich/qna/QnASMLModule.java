package edu.umich.qna;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.CountDownLatch;

import sml.Agent;
import sml.Identifier;
import sml.Kernel;
import sml.WMElement;
import sml.smlSystemEventId;
import sml.Agent.OutputEventInterface;
import sml.Kernel.SystemEventInterface;

public class QnASMLModule {
	
	final Identifier inputLink;
	final Agent agent;
	final long queryOutputHandle;
	final long disposeOutputHandle;
	final long nextOutputHandle;
	
	final DataSourceManager man;
	final Map<Integer, ResultSetInfo> resultSets;
	int maxCursor;
	
	CountDownLatch doneSignal;
	
	
	private class ResultSetInfo {
		public final String source;
		public final String queryName;
		public Identifier id;
		public QueryState queryState;
		
		ResultSetInfo(String source, String queryName, QueryState queryState) {
			this.source = source;
			this.queryName = queryName;
			this.queryState = queryState;
			
			id = null;
		}
	}
	
	public static final String QUERY_COMMAND_NAME = "qna-query";
	public static final String DISPOSE_COMMAND_NAME = "qna-dispose";
	public static final String NEXT_COMMAND_NAME = "qna-next";
	
	public static final String NEXT_NAME = "next";
	public static final String CURSOR_NAME = "cursor";
	public static final String RESULT_NAME = "result";
	public static final String QUERY_NAME = "query";
	public static final String SOURCE_NAME = "source";
	public static final String RESULTS_NAME = "results";
	public static final String INCREMENTAL_NAME = "incremental";
	public static final String PARAMETERS_NAME = "parameters";
	public static final String RESULTSET_NAME = "qna-resultset";
	public static final String CURSOR_NIL = "nil";
	
	public QnASMLModule(String host, int port, String agentName, DataSourceManager man, CountDownLatch doneSignal) {
		final Kernel kernel = Kernel.CreateRemoteConnection(true, host, port);
		if (kernel.HadError()) {
			throw new IllegalStateException(kernel.GetLastErrorDescription());
		}

		final Agent agent = kernel.GetAgent(agentName);
		if (agent == null) {
			throw new IllegalStateException(kernel.GetLastErrorDescription());
		}
		this.agent = agent;
		this.inputLink = agent.GetInputLink();
		
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_BEFORE_SHUTDOWN, shutdownHandler, null);

		queryOutputHandle = agent.AddOutputHandler(QUERY_COMMAND_NAME, queryCommandHandler, null);
		disposeOutputHandle = agent.AddOutputHandler(DISPOSE_COMMAND_NAME, disposeCommandHandler, null);
		nextOutputHandle = agent.AddOutputHandler(NEXT_COMMAND_NAME, nextCommandHandler, null);
		agent.SetOutputLinkChangeTracking(true);
		
		this.man = man;
		Identifier qnaRegistry = inputLink.CreateIdWME("qna-registry");
		for (Entry<String, Map<String, String>> s : man.getRegistry().entrySet()) {
			Identifier dataSource = qnaRegistry.CreateIdWME(s.getKey());
			
			for (Entry<String, String> q : s.getValue().entrySet()) {
				dataSource.CreateStringWME(QUERY_NAME, q.getKey());
			}
		}
		
		maxCursor = 1;
		resultSets = new HashMap<Integer, ResultSetInfo>();
		
		this.doneSignal = doneSignal;
	}
	
	public void close() {
		agent.RemoveOutputHandler(queryOutputHandle);
		agent.RemoveOutputHandler(disposeOutputHandle);
		agent.RemoveOutputHandler(nextOutputHandle);
	}
	
	private void addResult(QueryState queryState, Identifier id) {
		Map<String, List<Object>> row = queryState.next();
		
		for (Entry<String, List<Object>> c : row.entrySet()) {
			for (Object v : c.getValue()) {
				if (v instanceof Integer) {
					id.CreateIntWME(c.getKey(), ((Integer) v).intValue());
				} else if (v instanceof Double) {
					id.CreateFloatWME(c.getKey(), ((Double) v).doubleValue());
				} else {
					id.CreateStringWME(c.getKey(), v.toString());
				}
			}
		}
	}
	
	private void addResultSet(Integer cursorId, boolean incremental) {
		
		if (resultSets.containsKey(cursorId)) {
			ResultSetInfo rs = resultSets.get(cursorId);
			
			if (rs.id == null) {
				Identifier newResultSet = inputLink.CreateIdWME(RESULTSET_NAME);
				rs.id = newResultSet;
				
				newResultSet.CreateStringWME(SOURCE_NAME, rs.source);
				newResultSet.CreateStringWME(QUERY_NAME, rs.queryName);
				newResultSet.CreateIntWME(CURSOR_NAME, cursorId);
				
				Identifier resultId = newResultSet.CreateIdWME(RESULT_NAME);
				addResult(rs.queryState, resultId);
				
				if (incremental) {
					if (rs.queryState.hasNext()) {
						Integer nextCursor = new Integer(maxCursor++);
						
						resultSets.put(nextCursor, new ResultSetInfo(rs.source, rs.queryName, rs.queryState));
						resultId.CreateIntWME(NEXT_NAME, nextCursor.intValue());
					} else {
						resultId.CreateStringWME(NEXT_NAME, CURSOR_NIL);
					}
				} else {
					while (rs.queryState.hasNext()) {
						resultId = resultId.CreateIdWME(NEXT_NAME);
						addResult(rs.queryState, resultId);
					}
					resultId.CreateStringWME(NEXT_NAME, CURSOR_NIL);
				}
			}
		}
	}
	
	private final SystemEventInterface shutdownHandler = new SystemEventInterface() {

		@Override
		public void systemEventHandler(int eventID, Object data, Kernel kernel) {
			doneSignal.countDown();
		}
		
	};
	
	private final OutputEventInterface disposeCommandHandler = new OutputEventInterface() {

		@Override
		public void outputEventHandler(Object data, String agentName,
				String attributeName, WMElement pWmeAdded) {
			if (pWmeAdded.IsJustAdded() && pWmeAdded.IsIdentifier()) {
				WMElement cursorWme = pWmeAdded.ConvertToIdentifier().FindByAttribute(CURSOR_NAME, 0);
				boolean goodDispose = false;
				
				if ((cursorWme != null) && (cursorWme.ConvertToIntElement() != null)) {
					Integer cursorId = new Integer(cursorWme.ConvertToIntElement().GetValue());
					
					if (resultSets.containsKey(cursorId)) {
						ResultSetInfo rs = resultSets.get(cursorId);
						
						if (rs.id != null) {
							goodDispose = true;
							rs.id.DestroyWME();
							resultSets.remove(cursorId);
						}
					}
				}
				
				if (goodDispose) {
					pWmeAdded.ConvertToIdentifier().AddStatusComplete();
				} else {
					pWmeAdded.ConvertToIdentifier().AddStatusError();
				}
				
			}
		}
	
	};
	
	private final OutputEventInterface nextCommandHandler = new OutputEventInterface() {

		@Override
		public void outputEventHandler(Object data, String agentName,
				String attributeName, WMElement pWmeAdded) {
			if (pWmeAdded.IsJustAdded() && pWmeAdded.IsIdentifier()) {
				WMElement cursorWme = pWmeAdded.ConvertToIdentifier().FindByAttribute(CURSOR_NAME, 0);
				boolean goodNext = false;
				
				if ((cursorWme != null) && (cursorWme.ConvertToIntElement() != null)) {
					Integer cursorId = new Integer(cursorWme.ConvertToIntElement().GetValue());
					
					if (resultSets.containsKey(cursorId)) {
						ResultSetInfo rs = resultSets.get(cursorId);
						
						if (rs.id == null) {
							goodNext = true;
							addResultSet(cursorId, true);
						}
					}
				}
				
				if (goodNext) {
					pWmeAdded.ConvertToIdentifier().AddStatusComplete();
				} else {
					pWmeAdded.ConvertToIdentifier().AddStatusError();
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
				Integer cursorId = null;
				
				String uid = pIdAdded.GetParameterValue(SOURCE_NAME);
				String queryName = pIdAdded.GetParameterValue(QUERY_NAME);
				String results = pIdAdded.GetParameterValue(RESULTS_NAME);
				WMElement paramsWme = pIdAdded.FindByAttribute(PARAMETERS_NAME, 0);
				if ((paramsWme != null) && paramsWme.IsIdentifier()) {
					Identifier paramsId = paramsWme.ConvertToIdentifier();
					Map<Object, List<Object>> queryParams = new HashMap<Object, List<Object>>();
					
					for (int i=0; i<paramsId.GetNumberChildren(); i++) {
						WMElement childWme = paramsId.GetChild(i);
						
						if (!childWme.IsIdentifier()) {
							Object childAttr = null;
							try {
								childAttr = new Integer(Integer.parseInt(childWme.GetAttribute()));
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
								queryParams.get(childAttr).add(new Integer(childWme.ConvertToIntElement().GetValue()));
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
					} 
					if ((queryState != null)) {
						cursorId = new Integer(maxCursor++);
						
						resultSets.put(cursorId, new ResultSetInfo(uid, queryName, queryState));
						addResultSet(cursorId, results.equals(INCREMENTAL_NAME));
					}
				}
				
				if (cursorId == null) {
					pWmeAdded.ConvertToIdentifier().CreateStringWME(CURSOR_NAME, CURSOR_NIL);
				} else {
					pWmeAdded.ConvertToIdentifier().CreateIntWME(CURSOR_NAME, cursorId.intValue());
				}
			}
			
		}
		
	};
}
