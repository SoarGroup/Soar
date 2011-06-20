package edu.umich.soar.qna;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CountDownLatch;

import sml.Agent;
import sml.Agent.OutputEventInterface;
import sml.Identifier;
import sml.IntElement;
import sml.Kernel;
import sml.StringElement;
import sml.WMElement;

public class SynchronousSMLModule extends SMLModule {
	
	private final Map<Long, ResultState> intermediateResults = new HashMap<Long, ResultState>();
	private class ResultState {
		public final QueryState queryState;
		
		public Identifier oldParent;
		
		public long resultNum;
		public IntElement resultNumWME;
		public Identifier featuresId;
		public StringElement nextWME;
		
		ResultState(Identifier oldParent, QueryState queryState) {
			this.queryState = queryState;
			
			this.oldParent = oldParent;
			this.resultNum = 1;
			this.resultNumWME = null;
			this.featuresId = null;
			this.nextWME = null;
		}
	};
	
	public SynchronousSMLModule(Kernel kernel, Agent agent, DataSourceManager man, CountDownLatch doneSignal) {
		super(kernel, agent, man, doneSignal);
		registerCallbacks(queryCommandHandler, null, nextCommandHandler, null);
	}
	
	private void addResult(Long queryId, boolean incremental) {
		if (!intermediateResults.containsKey(queryId)) {
			return;
		}
		
		ResultState rs = intermediateResults.get(queryId);
		
		if (incremental) {
			if (rs.resultNum==1) {
				rs.oldParent = rs.oldParent.CreateIdWME(RESULT_NAME);
			} else {
				if (rs.resultNumWME!=null) {
					rs.resultNumWME.DestroyWME();
					rs.resultNumWME = null;
				}
				
				if (rs.featuresId!=null) {
					rs.featuresId.DestroyWME();
					rs.featuresId = null;
				}
			}
			
			rs.featuresId = rs.oldParent.CreateIdWME(FEATURES_NAME);
			rs.resultNumWME = rs.oldParent.CreateIntWME(NUM_NAME, rs.resultNum++);
			
			addFeatures(rs.featuresId, rs.queryState.next());
			
			if (rs.queryState.hasNext()) {
				if (rs.nextWME==null) {
					rs.nextWME = rs.oldParent.CreateStringWME(NEXT_NAME, PENDING_NAME);
				}
			} else {
				if (rs.nextWME!=null) {
					rs.nextWME.DestroyWME();
					rs.nextWME = null;
				}
				rs.oldParent.CreateStringWME(NEXT_NAME, NIL_NAME);
				
				intermediateResults.remove(queryId);
			}
		} else {
			Identifier oldParent = rs.oldParent;
			Identifier newParent = null;
			long roundNum = rs.resultNum;
			
			do {				
				if (roundNum==1) {
					newParent = oldParent.CreateIdWME(RESULT_NAME);
				} else {
					newParent = oldParent.CreateIdWME(NEXT_NAME);
				}
				
				addFeatures(newParent.CreateIdWME(FEATURES_NAME), rs.queryState.next());
				newParent.CreateIntWME(NUM_NAME, roundNum++);
				
				if (rs.queryState.hasNext()) {
					oldParent = newParent;
				} else {
					newParent.CreateStringWME(NEXT_NAME, NIL_NAME);
				}
			} while (rs.queryState.hasNext());
			
			intermediateResults.remove(queryId);
		}
	}
	
	private final OutputEventInterface nextCommandHandler = new OutputEventInterface() {
		
		public void outputEventHandler(Object data, String agentName, String attributeName, WMElement pWmeAdded) {
			if (pWmeAdded.IsJustAdded() && pWmeAdded.IsIdentifier()) {
				boolean goodNext = false;
				Identifier pIdAdded = pWmeAdded.ConvertToIdentifier();
				Long queryId = parseNext(pWmeAdded);
				
				if (queryId != null) {
					if (intermediateResults.containsKey(queryId)) {
						addResult(queryId, true);
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

		public void outputEventHandler(Object data, String agentName, String attributeName, WMElement pWmeAdded) {
			if (pWmeAdded.IsJustAdded() && pWmeAdded.IsIdentifier()) {
				boolean goodQuery = false;
				Identifier pIdAdded = pWmeAdded.ConvertToIdentifier();
				QueryCommandContents queryInfo = parseQuery(pWmeAdded);
				
				if (queryInfo != null) {
					goodQuery = true;
					
					QueryState queryState = null;
					try {
						queryState = man.executeQuery(queryInfo.uid, queryInfo.queryName, queryInfo.parameters);
					} catch (Exception e) {
						e.printStackTrace();
						goodQuery = false;
					} 
					
					if ((queryState != null)) {
						pIdAdded.CreateIntWME(ID_NAME, queryCounter);						
						intermediateResults.put(queryCounter, new ResultState(pIdAdded, queryState));
						
						addResult(queryCounter, queryInfo.results.compareTo(INCREMENTAL_NAME)==0);
						
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
