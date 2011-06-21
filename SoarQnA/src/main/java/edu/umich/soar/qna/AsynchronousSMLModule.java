package edu.umich.soar.qna;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.LinkedBlockingQueue;

import sml.Agent;
import sml.Agent.OutputEventInterface;
import sml.Identifier;
import sml.Kernel;
import sml.Kernel.AgentEventInterface;
import sml.Kernel.UpdateEventInterface;
import sml.WMElement;
import sml.smlAgentEventId;
import sml.smlUpdateEventId;

public class AsynchronousSMLModule extends SMLModule {
	
	public AsynchronousSMLModule(Kernel kernel, Agent agent, DataSourceManager man, CountDownLatch doneSignal) {
		super(kernel, agent, man, doneSignal);
		registerCallbacks(queryCommandHandler, null, nextCommandHandler, null);
		
		kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, updateHandler, agent);
		kernel.RegisterForAgentEvent(smlAgentEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED, initHandler, agent);
		
		meta = new HashMap<Long, QueryMetaData>();
		resultsFirst = new HashMap<Long, BlockingQueue<QueryResultDatum>>();
		resultsAll = new HashMap<Long, BlockingQueue<QueryResultDatum>>();
		resultsIncremental = new HashMap<Long, BlockingQueue<QueryResultDatum>>();
		resultsNext = new HashMap<Long, BlockingQueue<QueryResultDatum>>();
	}
	
	private void interruptThreads() {
		for (Map.Entry<Long, QueryMetaData> w : meta.entrySet()) {
			if (w.getValue().worker.getState()!=Thread.State.TERMINATED) {
				w.getValue().worker.interrupt();
			}
		}
	}
	
	public void close() {
		super.close();
		interruptThreads();
	}
	
	private final OutputEventInterface nextCommandHandler = new OutputEventInterface() {
		
		public void outputEventHandler(Object data, String agentName, String attributeName, WMElement pWmeAdded) {
			if (pWmeAdded.IsJustAdded() && pWmeAdded.IsIdentifier()) {
				boolean goodNext = false;
				Identifier pIdAdded = pWmeAdded.ConvertToIdentifier();
				Long queryId = parseNext(pWmeAdded);
				
				if (queryId != null) {
					if (resultsIncremental.containsKey(queryId)) {
						goodNext = true;
						resultsNext.put(queryId, resultsIncremental.remove(queryId));
						
						QueryMetaData m = meta.get(queryId);
						m.nextId = pIdAdded;
						m.nextStatus = pIdAdded.CreateStringWME(STATUS_NAME, PENDING_NAME);
					}
				}
				
				if (!goodNext) {
					pIdAdded.AddStatusError();
				}
			}
		}
	};
	
	private final OutputEventInterface queryCommandHandler = new OutputEventInterface() {

		public void outputEventHandler(Object data, String agentName, String attributeName, WMElement pWmeAdded) {
			if (pWmeAdded.IsJustAdded() && pWmeAdded.IsIdentifier()) {
				boolean goodQuery = false;
				Identifier pIdAdded = pWmeAdded.ConvertToIdentifier();
				QueryCommandContents queryInfo = parseQuery(pWmeAdded);
				
				if ((queryInfo != null) && man.validateQuery(queryInfo.uid, queryInfo.queryName)) {
					goodQuery = true;
					
					// create result queue, add to first
					BlockingQueue<QueryResultDatum> newQueue = new LinkedBlockingQueue<QueryResultDatum>();
					resultsFirst.put(queryCounter, newQueue);
					
					// create thread, associate with query + result queue, start!
					Thread newWorker = new Thread(new QueryWorker(queryInfo, newQueue));
					newWorker.start();
					
					// associate meta data
					meta.put(queryCounter, new QueryMetaData(pIdAdded, pIdAdded.CreateStringWME(STATUS_NAME, PENDING_NAME), newWorker, queryInfo.results.compareTo(INCREMENTAL_NAME)==0));
					
					pIdAdded.CreateIntWME(ID_NAME, queryCounter);
					queryCounter++;
				}
				
				if (!goodQuery) {
					pIdAdded.AddStatusError();
				}
			}			
		}		
	};
	
	private final AgentEventInterface initHandler = new AgentEventInterface() {
		
		public void agentEventHandler(int eventID, Object data, String agentName) {
			interruptThreads();
			meta.clear();
			resultsFirst.clear();
			resultsAll.clear();
			resultsNext.clear();
			resultsIncremental.clear();
		}
	};
	
	private final UpdateEventInterface updateHandler = new UpdateEventInterface() {
		
		public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
			//Agent agent = (Agent) data;
			
			// firsts go FIRST (sets up other queues)
			{
				Map<Long, Boolean> firsts = new HashMap<Long, Boolean>();
				
				QueryResultDatum d = null;
				for (Map.Entry<Long, BlockingQueue<QueryResultDatum>> e : resultsFirst.entrySet()) {
					if (!e.getValue().isEmpty()) {
						d = e.getValue().peek();
						firsts.put(e.getKey(), ((d!=null) && (d.features!=null)));
					}
				}
				
				BlockingQueue<QueryResultDatum> q;
				QueryMetaData m;
				for (Map.Entry<Long, Boolean> f : firsts.entrySet()) {
					q = resultsFirst.remove(f.getKey());
					m = meta.get(f.getKey());
					
					// pending is nixed
					if (m.status!=null) {
						m.status.DestroyWME();
						m.status = null;
					}
					
					if (f.getValue()) {
						m.cmdRoot.AddStatusComplete();
						m.resultId = m.cmdRoot.CreateIdWME(RESULT_NAME);
						
						if (m.incremental) {
							resultsNext.put(f.getKey(), q);
						} else {
							resultsAll.put(f.getKey(), q);
						}
					} else {
						m.cmdRoot.AddStatusError();
						meta.remove(f.getKey());
					}
				}
			}
			
			// all
			{
				QueryMetaData m = null;
				QueryResultDatum d = null;
				List<Long> lasts = new LinkedList<Long>();
				
				for (Map.Entry<Long, BlockingQueue<QueryResultDatum>> r : resultsAll.entrySet()) {
					if (!r.getValue().isEmpty()) {
						m = meta.get(r.getKey());
					}
					
					while (!r.getValue().isEmpty()) {
						try {
							d = r.getValue().take();
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
						
						if (m.nextWme!=null) {
							m.nextWme.DestroyWME();
							m.nextWme = m.resultId.CreateIdWME(NEXT_NAME);
						} else {
							m.nextWme = m.resultId;
						}
						
						addFeatures(m.nextWme.ConvertToIdentifier().CreateIdWME(FEATURES_NAME), d.features);
						m.nextWme.ConvertToIdentifier().CreateIntWME(NUM_NAME, m.resultNum++);
						
						if (d.last) {
							lasts.add(r.getKey());
							m.nextWme.ConvertToIdentifier().CreateStringWME(NEXT_NAME, NIL_NAME);
						} else {
							m.resultId = m.nextWme.ConvertToIdentifier();
							m.nextWme = m.resultId.CreateStringWME(NEXT_NAME, PENDING_NAME);
						}
					}
				}
				
				for (Long k : lasts) {
					meta.remove(k);
					resultsAll.remove(k);
				}
			}
			
			// next
			{
				QueryMetaData m = null;
				QueryResultDatum d = null;
				List<Long> incs = new LinkedList<Long>();
				List<Long> dones = new LinkedList<Long>();
				
				for (Map.Entry<Long, BlockingQueue<QueryResultDatum>> r : resultsNext.entrySet()) {
					if (!r.getValue().isEmpty()) {
						m = meta.get(r.getKey());
						try {
							d = r.getValue().take();
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
						
						if (m.resultNumWme!=null) {
							m.resultNumWme.DestroyWME();
							m.resultNumWme = null;
						}
						
						if (m.featuresId!=null) {
							m.featuresId.DestroyWME();
							m.featuresId = null;
						}
						
						m.featuresId = m.resultId.CreateIdWME(FEATURES_NAME);
						m.resultNumWme = m.resultId.CreateIntWME(NUM_NAME, m.resultNum++);
						
						addFeatures(m.featuresId, d.features);
						
						if (d.last) {
							if (m.nextWme!=null) {
								m.nextWme.DestroyWME();
								m.nextWme = null;
							}
							
							m.resultId.CreateStringWME(NEXT_NAME, NIL_NAME);
							
							dones.add(r.getKey());
						} else {
							if (m.nextWme==null) {
								m.nextWme = m.resultId.CreateStringWME(NEXT_NAME, PENDING_NAME);
							}
							
							incs.add(r.getKey());
						}
						
						if (m.nextId!=null) {
							if (m.nextStatus!=null) {
								m.nextStatus.DestroyWME();
								m.nextId.AddStatusComplete();
								
								m.nextStatus = null;
							}
							
							m.nextId = null;
						}
					}
				}
				
				for (Long k : incs) {
					resultsIncremental.put(k, resultsNext.remove(k));
				}
				
				for (Long k : dones) {
					resultsNext.remove(k);
					meta.remove(k);
				}
			}
		}
	};
	
	//
	
	private class QueryWorker implements Runnable {

		public void run() {
			QueryState queryState = null;
			boolean interrupted = false;
			
			try {
				queryState = man.executeQuery(queryInfo.uid, queryInfo.queryName, queryInfo.parameters);
			} catch (InvalidDataSourceUIDException e) {
				e.printStackTrace();
				queryState = null;
			} catch (InvalidQueryUIDException e) {
				e.printStackTrace();
				queryState = null;
				interrupted = true;
			}
			
			if (!interrupted) {
				try {
					if ((queryState!=null) && (queryState.hasNext())) {
						do {
							resultQueue.put(new QueryResultDatum(queryState.next(), !queryState.hasNext()));
						} while (queryState.hasNext());
					} else {
						resultQueue.put(new QueryResultDatum(null, true));
					}
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			
			queryState.dispose();
		}
		
		public QueryWorker(QueryCommandContents queryInfo, BlockingQueue<QueryResultDatum> resultQueue) {
			this.queryInfo = queryInfo;
			this.resultQueue = resultQueue;
		}
		
		private final QueryCommandContents queryInfo;
		private final BlockingQueue<QueryResultDatum> resultQueue;
	};
	
	private class QueryResultDatum {
		public Map<String, List<Object>> features;
		boolean last;
		
		public QueryResultDatum(Map<String, List<Object>> features, boolean last) {
			this.features = features;
			this.last = last;
		}
		
		public String toString() {
			return ("("+last+"): "+features.toString());
		}
	};
	
	private class QueryMetaData {
		public Identifier cmdRoot;
		public WMElement status;
		public Thread worker;
		public boolean incremental;
		
		public long resultNum;
		public Identifier resultId;
		public WMElement nextWme;
		public Identifier featuresId;
		public WMElement resultNumWme;
		
		public Identifier nextId;
		public WMElement nextStatus;
		
		public QueryMetaData(Identifier cmdRoot, WMElement status, Thread worker, boolean incremental) {
			this.cmdRoot = cmdRoot;
			this.status = status;
			this.worker = worker;
			this.incremental = incremental;
			
			this.resultNum = 1;
			this.resultId = null;
			this.nextWme = null;
			this.featuresId = null;
			this.resultNumWme = null;
			
			this.nextId = null;
			this.nextStatus = null;
		}
	};
	
	private Map<Long, QueryMetaData> meta;
	private Map<Long, BlockingQueue<QueryResultDatum>> resultsFirst;
	private Map<Long, BlockingQueue<QueryResultDatum>> resultsAll;
	private Map<Long, BlockingQueue<QueryResultDatum>> resultsIncremental;
	private Map<Long, BlockingQueue<QueryResultDatum>> resultsNext;
	
}
