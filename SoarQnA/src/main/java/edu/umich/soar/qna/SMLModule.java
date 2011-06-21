package edu.umich.soar.qna;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.CountDownLatch;

import sml.Agent;
import sml.Agent.OutputEventInterface;
import sml.Identifier;
import sml.IntElement;
import sml.Kernel;
import sml.Kernel.SystemEventInterface;
import sml.WMElement;
import sml.smlSystemEventId;

public abstract class SMLModule {
	protected Kernel kernel;
	protected Agent agent;
	
	protected final Identifier inputLink;
	
	protected Identifier qnaRegistry;
	
	protected final DataSourceManager man;
	
	protected CountDownLatch doneSignal;
	
	protected long queryCounter;
	
	protected static final String QUERY_COMMAND_NAME = "qna-query";
	protected static final String NEXT_COMMAND_NAME = "qna-next";
	
	protected static final String NEXT_NAME = "next";
	protected static final String RESULT_NAME = "result";
	protected static final String QUERY_NAME = "query";
	protected static final String SOURCE_NAME = "source";
	protected static final String RESULTS_NAME = "results";
	protected static final String INCREMENTAL_NAME = "incremental";
	protected static final String PARAMETERS_NAME = "parameters";
	protected static final String NIL_NAME = "nil";
	protected static final String FEATURES_NAME = "features";
	protected static final String PENDING_NAME = "pending";
	protected static final String ID_NAME = "id";
	protected static final String NUM_NAME = "num";
	protected static final String STATUS_NAME = "status";
	
	protected SMLModule(Kernel kernel, Agent agent, DataSourceManager man, CountDownLatch doneSignal) {
		this.kernel = kernel;
		this.agent = agent;
		this.inputLink = agent.GetInputLink();
		
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
	
	protected void registerCallbacks(OutputEventInterface queryCommandHandler, Object queryData, OutputEventInterface nextCommandHandler, Object nextData) {
		kernel.RegisterForSystemEvent(smlSystemEventId.smlEVENT_BEFORE_SHUTDOWN, shutdownHandler, null);
		agent.AddOutputHandler(QUERY_COMMAND_NAME, queryCommandHandler, queryData);
		agent.AddOutputHandler(NEXT_COMMAND_NAME, nextCommandHandler, nextData);
	}
	
	public void close() {
		qnaRegistry.DestroyWME();
	}
	
	protected void addFeatures(Identifier features, Map<String, List<Object>> row) {
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
	
	protected Long parseNext(WMElement pWmeAdded) {
		Long returnValue = null;

		if (pWmeAdded.IsJustAdded() && pWmeAdded.IsIdentifier()) {
			Identifier pIdAdded = pWmeAdded.ConvertToIdentifier();

			WMElement queryWme = pIdAdded.FindByAttribute(QUERY_NAME, 0);
			if (queryWme!=null) {
				IntElement queryInt = queryWme.ConvertToIntElement();
	
				if (queryInt != null) {
					returnValue = queryInt.GetValue();
				}
			}
		}

		return returnValue;
	}
	
	protected class QueryCommandContents {
		public final String uid;
		public final String queryName;
		public final String results;
		public final Map<Object, List<Object>> parameters;
		
		QueryCommandContents(String uid, String queryName, String results, Map<Object, List<Object>> parameters) {
			this.uid = uid;
			this.queryName = queryName;
			this.results = results;
			this.parameters = parameters;
		}
	}
	
	protected QueryCommandContents parseQuery(WMElement pWmeAdded) {
		QueryCommandContents returnValue = null;
		
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
				
				returnValue = new QueryCommandContents(uid, queryName, results, queryParams);
			}
		}
		
		return returnValue;
	}
	
	protected final SystemEventInterface shutdownHandler = new SystemEventInterface() {

		public void systemEventHandler(int eventID, Object data, Kernel kernel) {
			doneSignal.countDown();
		}
		
	};
	
}
