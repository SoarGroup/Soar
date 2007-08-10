/ Approximate Graph Isomorphism.
/ graph.k
/ To do: implement ullmann: create a matrix 
/ (#nodes of graph1) x (#nodes of graph 2)
/ of 1s and 0s where 1 in position
/ i,j means that degree of node i in graph 1 <= degree of node j in graph 2.
/ and types are compatible.
/ Then eliminate 1s so every row has one 1 and every column has one 1.
/ Change this for subgraph isomorphism appropriately.

/ Begun 1999 and continuing into 2000
/ Joint Work with Jason Wang and Kaizhong Zhang
/ Author: Dennis Shasha
/ This does graph to subgraph isomorphism and a fair job at subgraph
/ to subgraph isomorphism.

/ Multiedges or general labels on edges with general sets of labels.
/ Weight of differences is the size of the set difference.
/
/ To do: For graph display, let's try the following.
/ Each graph is displayed using some graph display package that
/ can display nodes in color with labels.
/ Display each graph in a color corresponding to its type and then
/ use the labels to indicate mapping align, e.g. if node 0 from graph g1
/ maps to node 5 in g2, then these will be both labeled A; if node 1 maps
/ to 3, they would be labeled B and so on.
/ To do: try not to use countrowpure so much by computing valences in advance.
/ To do: to cluster graphs. Take all the pairwise similarities and
/ specify a minimum similarity. 
/ Then try to find the smallest number of graphs that are that similar
/ to every other one above that minimum similarity.
/ Do this by starting with the graph that has the most neighbors
/ above this level of similarity, then continue trying to get as many
/ good neighbors as possible.
/ Done. April 7, 2000

/ To do: find best minimum graph. Suppose that when you compute
/ the best similarity between two graphs, you also write out the edges
/ in the first graph that are different.
/ If a given graph g is different from g1, ... gn
/ remove edges from g in proportion to the number of those other
/ graphs that removal will help.
/ But you must leave the graph connected.
/ Not so clear this is desirable but if it is, then
/ strategy is to do clustering first and then to 
/ treat each cluster as a probe vs. database situation.
/ Within each cluster, we  calculate the alignment
/ from the source label to the target labels and then
/ find the intersection of the source nodes and
/ then try to find the smallest graph containing
/ those source nodes and then remeasure the distances.
/ Everything is done as of April 8, 2000 except remeasuring the distances.

/ Timing on sparc 5:  (manygraphs is the number of graphs if positive)
/ Here edgecount = 2 * nodecount
/ 100 graphs of size (nodecount) 400. 735 seconds.
/ 100 graphs of size (nodecount) 200. 207 seconds.
/ 100 graphs of size (nodecount) 100. 100 seconds.
/ 100 graphs of size (nodecount) 50. 37 seconds.
/ Here edgecount = 4 * nodecount
/ 100 graphs of size (nodecount) 200. 355 seconds.

/ Here edgecount = 20*nodecount
/ 50 graphs of size (nodecount) 50. 180 seconds
/ Here edgecount = 10*nodecount
/ 50 graphs of size (nodecount) 50. 90 seconds


/ March, 2000: made filtering a little more permissive for the sake
/ of greater accuracy. This can be adjusted with the parameter minsim.
/ Dec. 1999: took out the calcneighvalencecount stuff. Not helpful.
/ Also, incorporated the following tests in vallessthanequal: 
/ node n in subgraph
/ g1 can match a node m in subgraph g2 if 
/ valence(n) <= valence(m) and valence(neighs(n)) <= valence(neighs(m))
/ and types(neighs(n)) is a subbag of types(neighs(m)).
/ This last condition is new, but it helps a bit.
/ It might be overly selective.
/ I also had hoped that doing this new test would render
/ hillclimbing unnecessary, but it did not.
/ The good news is that hillclimbing is quite cheap.
/ Possible to do: instead of putting a node into extra, first
/ check whether just the subbag relationship holds.
/ This doesn't seem to help, but it doesn't hurt either, 
/ so I'm leaving in anyway.
/ One thing I've taken out is the attemptswap call. It doesn't seem to help.
/ Note that if we ever have a situation in which we incorporate new features
/ e.g. real distances or different relationships among types,
/ we can put all that intelligence in 
/ vallessthanequal and cheapvallessthanequal.

/ Format: k graph [nodecount [maxnumpairs [lower [upper [extraedges [extranodes[heuristic]]]]]]]

/ givemealignment: 1 / if 1, produce alignment for output
rejectlabels: () / initialization
realorigedgeall: () / initialized for case with multiedges
realoriglabel: ()
multiedgeflag: 0 / set to 1 if we ever get to something with multiedges

/ These are other parameters beyond those on the command line.
/ If manygraphs is positive, that tells how many random graphs to generate
manygraphs: 4  / if 0 just compare adj1 with adj2. Otherwise, generate
/ the number manygraphs of random graphs and do a global comparison.
/ To modify this so that we are given a series of graphs as a database
/ and a new graph as a test, set manygraphs to -1.
/ In that case, the input graphs of databases 
/ come from tempin (filenameforalltoall)
/ where they are laid out or produced as needed.
/ Distances can be greater than 1 which means the edges are less strong.
/ manygraphs = -2 means take input from file tempin 
/ (filenameforalltoall) and find best match
/ of each graph against all others
/ and then if kmeans > 0, try to find the best kmeans source graphs
/ that are most similar to all the others
/ manygraphs = -3 means that there are two files: "tempindb" (dbfile)
/ and "tempinprobes" (probefile); THIS IS CLASSIC DATABASE QUERY SETUP
/ We execute one probe at a time.
/ manygraphs = -4 UNUSED
/ manygraphs = -5 means we have one file tempin (filenameforalltoall)
/  and we will pick
/ some of those to be probes and others not to be

/ kmeans > 0 means that we want that many centroids
/ (works when manygraphs = -2)
reducecentroids: 0
/ reducecentroids = 1 says to find subgraphs of centroids that are
/ most essential to the mapping from the centroid to the other graphs.
/ happens only when kmeans > 0

/ TIME TESTING HARNESS (REMOVE / if you want this)
/ \l time

/ hillclimbing: 1 / if 1 then do hill climbing, if 0, then do not

/ numtypes: 2 / all nodes belong to the same type if there is one type.
/ When the number of types is k, then each node can match only
/ other nodes of the same type.


/ To do:
/ 1. In checkalign, look to make sure that types are correct before saying
/ that an edge is consistent. (This is moot if the alignment guarantees this,
/ because then {i,j} in E and {m[i], m[j]} in E2 imply that types1[i] =
/ types2[m[i]] and types2[j] = types2[m[j]].).
/ Because of this, the way we count valdist in a type-dependent way 
/ works well. Note that this is a bit of overhead.
/ Done. August 23, 1999.
/ 2. Long edges: Represent as bigger integers.
/ Idea is that if an edge is weak, then its strength is smaller
/ and that is the case if the distance is larger.
/ That is what we do for geometric hashing.
/ checkalign gives a vote that is 1 if strengths are the same,
/ and the smaller of strength1/strength2 if the strengths are different.
/ countrow takes the inverse of the distance since edge strength
/ is less if distance is bigger.
/ All the valence strategies interpret the strength of a valence
/ as the inverse of its distance values.
/ Done: August 29, 1999.
/ 3. test zoom by having vastly more nodes and edges
/ Quality criterion changes: strength of edges in large
/ graph must be as large as possible.
/ 4. figure out how to do multiple alignment
/ This means the common subgraphs of each alignment.
/ Maybe have many graphs try to align to a single centroid graph,
/ using a subgraph match and take the intersection of all of those
/ matches to the centroid graph.
/ 5. Figure out how to do subgraph matching.
/ Cost of a subgraph mapping is the cost of just those nodes in the
/ mapping. An A* style approach is to posit a mapping and evaluate its
/ cost and then extend one mapping to get the best next mapping.




heuristic: 4 / if we set this to zero, then we use randomalignment.
/ When we set this to 1 we use heuristic 1 (by degree).
/ When we set this to 3 we use heuristic 3 (geometric hashing).
/ When we set to 4, we are using valence of the neighbors as well.
/ When we set to 5, we are using the Ullmann method modified for types.

/ We want good heuristics for approximate graph isomorphism
/ and approximate subgraph isomorphism.
/ Our distance measure will be based on the fraction of distances
/ that are correct.


/ We want to use geometric hashing for node and edge style graphs.
/ So, we have a notion of point and a notion of distance which is
/ the minimum distance between two points.
/ We also have an adjacency matrix.
/ As in geometric hashing for strings, we will hash small areas
/ by changing distances.



/ Experimental Setup:
/ i) Data Generation.
/ Generate a random graph G1 having n nodes and e edges (in these preliminary
/ experiments we studied e = 2*n and e = 4*n),
/ then relabel the n nodes based on a random permutation producing
/ the isomorphic graph G2.
/ Add some number of nodes and edges to G2, thus causing G2 to
/ be a supergraph of G1.
/ 
/ ii) Find Candidate Isomorphism.
/ Using heuristics,
/ find a mapping m of nodes in G1 to nodes in G2
/ (an injective but not surjective function in the case that G2
/ is a proper supergraph of G1).
/ 
/ iii) Evaluate Quality.
/ Test how close to an isomorphism the mapping is.
/ The quality of a mapping is the fraction of edges in G1
/ that are consistent with the isomorphism, i.e. {i, j} in G1 is a consistent edge
/ using m if {m[i], m[j]} is an edge in G2.

/ Heuristic 1: 
/ i) Rank the nodes in G1 and G2 in ascending order by their degree,
/ yielding two partially ordered lists of nodes PartOrd1 and PartOrd2.
/ They are partially ordered because many nodes in each graph will
/ have the same degree.
/ We call each set of nodes from a single graph having the same degree 
/ an equivalence class.
/ ii) Construct a large number (based on a parameter) of total orders,
/ TotOrd1 and TotOrd2, from these partially orderd sets
/ based on ramdom permutations of the equivalence classes.
/ Each pair (TotOrd1, the prefix of TotOrd2 of length |TotOrd1|) 
/ defines an injective function and therefore is a candidate mapping.
/ iii) Choose the best mapping, based on the quality criterion defined in
/ the Experimental Setup step iii.
/ 
/ Heuristic 2: Use geometric hashing.
/ i) Find the all pairs shortest distance between every pair of nodes 
/ in G1 and G2.
/ ii) Choose a large number of reference sets (based on a parameter)
/ in each graph. A reference set is, in the current implementation,
/ a pair of nodes
/ that are at least {\em lower} distance apart and at most 
/ {\em upper} distance apart,
/ for two parameters {\em lower} and {\em upper}.
/ (If lower = upper = 0, then the pair reduces to a single point.
/ The best setting appears to be lower = upper = 1, corresponding
/ to the intuition from Wolfson and Ruth....)
/ iii) For each such reference set in each graph, 
/ compute the distances from each node
/ to each node of the 
/ reference set.
/ iv) Find the reference sets (r1, r2), where r1 is from G1 and 
/ r2 is from G2, receiving the most
/ node votes. A node vote for (r1, r2) consists of a pair of nodes 
/ n1 from G1 and n2 from G2 such that n1 has the same distance
/ from the nodes of r1 as n2 does for the nodes of r2.
/ (Ensure that each node n1 contributes
/ at most one vote to (r1, r2).) 
/ v) For each pair (r1, r2) receiving the highest number of votes,
/ denoted bestpairs (and having the closest average distance)
/ compute a mapping (i.e. a candidate isomorphism)
/ as follows:
/ v.a) Order all nodes in G1 based on their distances from r1
/ and all nodes in G2 based on their distances from r2.
/ There will be some equivalence classes in each case.
/ v.b) Choose an arbitrary permutation of each equivalence class. 
/ v.c) This gives a total order TotOrd1 of the nodes in G1
/ and a total order TotOrd2 of the nodes in G2.
/ vi) Choose the best mapping from the mappings
/ produced from bestpairs, based on the quality criterion defined in
/ the Experimental Setup step iii.
/ 
/ Heuristic 3: Combine geometric hashing with degree matching.
/ This is the same as heuristic 2 except that step v.b
/ is replaced by a step that orders the nodes of each equivalence class
/ based on their degree and creates the mapping based on degree
/ as in heuristic 1.
/ 
/ The results of the experiments are as follows.
/ 1. The quality of all heuristics degrade as the graph size is increased,
/ but have far better quality than a random mapping.
/ 2. Heuristics 2 and 3 have a time complexity proportional
/ to $n^3$ where $n$ is the number of nodes
/ in the graph, because the number of close pairs
/ rises linearly with the size of the graph and the computation of votes
/ to each pair is quadratic in the size of the graph.
/ Heuristic 2 is less than twice as fast as heuristic 3. 
/ Heuristic 1 is the fastest, since it is proportional to $n^2$
/ 3. Heuristic 3 gives the best results by far, able to discover
/ perfect isomorphisms up to size 20 and very good results thereafter.
/ 

/ ARGUMENTS


filenameforalltoall: "tempin"
probefile: "tempinprobes"
dbfile: "tempindb"
givemealignment: 0
manygraphs: -2
nodecount: 50
edgecount: -10 / impossible value
extranodes: 0
extraedges: 0
hillclimbing: 1
maxnumpairs: 50
numtypes: 2
kmeans: 1 / should this be 0?
okflag: 1
processargs:{[args]
  inputchoice: 0
  i: 0
  while[(i < #args) & okflag
	okflag:: 0
	if[args[i] ~ "+help"
		okflag:: 0
	]
	if[args[i] ~ "+f"
		filenameforalltoall:: args[i+1]
		manygraphs:: -2
		inputchoice+: 1
		okflag:: 1
	] 
	if[args[i] ~ "+nc"
		kmeans:: 0 $ args[i+1]
		okflag:: 1
      	]
	if[args[i] ~ "+p"
		probefile:: args[i+1]
		manygraphs:: -3
		inputchoice+: 1
		okflag:: 1
	]
	if[args[i] ~ "+d"
		dbfile:: args[i+1]
		manygraphs:: -3
		okflag:: 1
	]
	if[args[i] ~ "+m"
		manygraphs:: 0 $ args[i+1]
		inputchoice+: 1
		okflag:: 1
	] 
	if[args[i] ~ "+n"
		nodecount:: 0 $ args[i+1]
		okflag:: 1
	] 
	if[args[i] ~ "+t"
		numtypes:: 0 $ args[i+1]
		okflag:: 1
	] 
	if[args[i] ~ "+en"
		extranodes:: 0 $ args[i+1]
		okflag:: 1
	] 
	if[args[i] ~ "+ee"
		extraedges:: 0 $ args[i+1]
		okflag:: 1
	] 
	if[args[i] ~ "+a"
		givemealignment:: 0 $ args[i+1]
		okflag:: 1
	]
	if[args[i] ~ "+h"
		maxnumpairs:: 0 $ args[i+1]
		hillclimbing:: 0 < maxnumpairs
		okflag:: 1
	]
	i+: 2
	if[(0 = okflag) | (1 < inputchoice)
		x:"format is k graph \n"
		x,: "  (+f filenameforalltoall (tempin) [+nc numclusters (1)] | \n"
		x,: "   +p probefile (tempinprobes) +d dbfile (tempindb) | \n"
		x,: "   +m manygraphs (-2) [+n nodecount (50)] \n"
		x,: "        [+en extranodes (0)] [+ee extraedges (0)] [+t numtypes (2)]\n"
		x,: "  ) \n "
		x,: " [+a givemealignment (1)] "
		x,: " [+h numberofhillclimbs (50)] \n"
		x,: " 	[+help] \n"
		x,: " Example 1: All to all comparison of file tempin with 100 hillclimbs. \n"
		x,: " k graph +f tempin +h 100 \n"
		x,: " Example 2: five 70 node random graphs and lab2 with 8 extra nodes and 20 extra edges. \n"
		x,: " k graph +m 5 +n 70 +en 8 +ee 20 \n"
		` 0: x
		. "\\\\"
	]
  ]
}
processargs[_i]





/ SECTION: OLD WRITEUP

/ What are we doing when we do geometric hashing for strings?
/ We make use of the total order of strings.
/ We create hashes of small consecutive subsequences of strings as well
/ as of neighbors of those subsequences.
/ We then compare these hashes and perform votes to establish
/ best starting positions.
/ The votes depend critically on the total order because we are saying
/ i' matches j' which gives a vote to i'-k, j'-k for all k.
/ One thing that can go wrong: if there are two insertions in the middle of s1,
/ but otherwise s1 and s2 are equal, we won't notice that s1 and s2
/ are close if the neighbors
/ of subsequences are only one apart.
/ Note that we could have estimates of average distance to help us.
/
/ How does this compare with the situation for graphs?
/ In graphs, we have no total order. This still allows us to vote
/ however.
/ In this case the i' and j' are points and we are voting for pairs.
/ The question though is: if pair p1 and p2 are good, but not great,
/ then can a simple change make p1 and p2 great?
/ That is, let S1 and S2 be the sets that p1 and p2 match.
/ Let S1' and S2' be the points that are unmatched.
/ If some other pair p1' and p2' matches S1' and S2', then perhaps these two
/ can be put together.
/ The obvious thing to do is this: if S1' is closer to p1 then S2'
/ is to p2, then add some edges in graph G2. If farther, then remove some edges
/ from that graph. If equal, then we may have a disconnected graph.
/ Maybe we don't have to worry about this situation.
/ We just try to find a covering of local isomorphisms that
/ is not too big.
/ If we find that two isomorphisms is necessary, then distance is at least one.
/ If we find that k are necessary, then distance is at least k-1?
/ How much does a pair cover? It covers both the number that agree in
/ the multiset and the envelope of all of those that could agree in the 
/ multiset, e.g. if we have { (n1, (2, 1)), (n2, (2, 1)), (n3, (3,1))}
/ for p1 and  { (m6, (2, 1)), (m2, (2, 2)), (m5, (3,1))} for p2
/ then we match two in the multiset but n1, n2, n3 are all in the
/ envelope (to avoid maintaining many set-set pairs as the result
/ of each match).


/ PARAMETERS WITHIN PROGRAM


/ SECTION: BASICS

ceil:{[x] :[x = _ x; _ x; 1 + _ x]}

avg: {(+/x)%#x}

/ get rid of blanks at either end of the string
delendblanks:{[string]
  if[0 = #string; :""]
  if[string ~ ,"" ; :""]
  i: & ~ string = " "
  if[(#string) = (#i); :string]
  if[0 = (#i); :""]
  string: (- ((#string)  - (1 + *|i))) _ string
  :(*i) _ string
}


/ returns one if x is a subset of y
subset:{[x;y] (#y) > |/ y ?/: x}

/ returns one if x is a subbag (submultiset) of y
subbag:{[x;y] 
  uniqx: ?x
  uniqy: ? y
  / if[~ subset[uniqx;uniqy]; :0]
  jj: uniqy ?/: uniqx / find the ys corresponding to the xs
  if[(#uniqy) = |/jj; :0] / not a subset
  countx: #:'= x
  county: #:'= y
  zz: &/ ~  countx > county[jj] / make sure the ys dominate
  :zz
}


/finds intersection of two lists
/ fastest of all
intersect: {[x;y]
  x,: ()
  y,: ()
  i: x ?/: y
  :x[(?i) _dv #x]
}

/ this is a set intersection so we remove duplicates
multiintersect:{[lists]
 size: #lists
 if[2 > size; :lists]
 first: lists[0],()
 jj:  first ?/: (,/ ?:' lists[1+ !(size-1)]) / find indexes in first
 x: @[(1+#first) # 0; jj; + ; 1]
 x: (-1) _ x / delete missing entry
 kk: & x = size - 1
 :first[kk]
}

/finds intersection of two lists
/ fastest of all
hasintersect: {[x;y]
  x,: ()
  y,: ()
  i: x ?/: y
  :(#x) > &/i
}

/finds intersection of two lists
/ and returns index pairs of matches. Assumes no duplicates
/ in either list
intersectboth: {[x;y]
  x,: ()
  y,: ()
  i: x ?/: y
  pairs: (i ,' (!#y))
  if[0 = #pairs; :()]
  k: & pairs[;0] < #x
  :pairs[k]
}

/finds indexes in x that intersect with y
intersectleftindexes: {[x;y]
  i: x ?/: y / where each y hits
  j: & i < #x / those ys that hit
  :i[j]
}


/ A faster difference
differ:{[x;y]
  x,: ()
  y,: ()
  i: y ?/: x
  j: & i = #y
  :?x[j]
}

/ A faster difference, returns indexes
differleftindexes:{[x;y]
  x,: ()
  y,: ()
  i: y ?/: x
  j: & i = #y
  :j
}


/ SECTION: DISTANCE ESTIMATORS FOR FILTERS
/ Assuming the difference is the number of nodes and edges
/ that have changed, we can look at missing valences as a minimum
/ number of changes.

/ Number of elements of types1 that are not in types2, 
/ thought of as a multiset.
/ The return value here is the minimum distance of a graph to subgraph match.
/ SUPERSEDED by valtypedist
typedist:{[types1; types2]
  cpart1: #:' = types1
  cpart2: #:' = types2
  t1: ? types1
  t2: ? types2
  j: t2 ?/: t1
  kin: & j < #t2
  kout: & j = #t2
  tot: +/cpart1[kout]
  x: cpart1[kin] - cpart2[j[kin]]
  i: & x > 0
  tot+: +/ x[i]
  :tot
}

/ vals1 are the valences of nodes in graph 1 and vals2 are the valences
/ in graph 2. 
/ We want each node i of valence vi in graph 1 to be associated
/ with a node j of valence vj in graph 2 such that vi <= vj.
/ For each node of vals1 for which this is not possible,
/ ascribe a distance of 1. This may be too low if we are measuring
/ the number of different edges.
valdistold:{[vals1; vals2]
  v2: vals2[<vals2]
  count: @[((#v2) # -1), 0; v2 _bin/: vals1; +; 1]
  / count represents the demand on v2
  / A -1 means that a position can be assigned (supply).
  / A positive number represents excess demand.
  x: +\ |count / any positive number in x represents unmet excess demand
  max: |/x / the max represents the unmet demand, i.e. the differences
  / y: :[0 < max; ceil[max % 2]; 0]
  y: :[0 < max; ceil[max]; 0] / willing to double-count the distance
	/ because that is what we do later.
	/ So, the distance is number of half-edges that are different.
  :y
}

/ vals1 are the valences of nodes in graph 1 and vals2 are the valences
/ in graph 2. 
/ We want each node i of valence vi in graph 1 to be associated
/ with a node j of valence vj in graph 2 such that vi <= vj.
/ If it isn't then at least vi - vj edges from vi are going to be
/ wrong.
/ Simply add all of those up.
valdist:{[vals1; vals2]
  v1: | vals1[<vals1] / sorted in descending order
  v2: | vals2[<vals2] / sorted in descending order
  if[(#v1) < (#v2); v2@: !#v1]
  y: 0
  if[(#v2) < (#v1) 
	y+: +/ (#v2) _ v1 / add in valences of untouched nodes
	v1@: !#v2
  ]
  x: v1 - v2
  i: & x > 0
  y+: +/ x[i]
  :y
}


/ partitions valences by type and then adds up all the remainders
/ Lower bound on distance
valtypedist:{[vals1; vals2; types1; types2]
  part1: = types1
  part2: = types2
  p1uniq: ? types1
  p2uniq: ? types2
  y: differleftindexes[p1uniq; p2uniq]
  tot: +/ #:' part1[y]
  x: intersectboth[p1uniq; p2uniq] / just get indexes in part1 and part2
  i: 0
  while[i < #x
	tot+: valdist[vals1[part1[x[i;0]]] ; vals2[part2[x[i;1]]] ]
	i+: 1
  ]
  :tot
}

/ partitions valences by type and sees whether valences of first are low 
/ enough.
valtypedistnew:{[vals1; vals2; types1; types2]
  part1: = types1
  part2: = types2
  p1uniq: ? types1
  p2uniq: ? types2
  y: differleftindexes[p1uniq; p2uniq]
  tot: +/ #:' part1[y] / number of nodes that are different
  x: intersectboth[p1uniq; p2uniq] / just get indexes in part1 and part2
  i: 0
  while[i < #x
	v2: vals2[part2[x[i;1]]]
	v1: vals1[part1[x[i;0]]]
	tv2: v2 @ > v2	
	tv1: v1 @ > v1
	minsize: (#v1) & (#v2)
	tv2@: !minsize
	tv1@: !minsize
	tot+: ((#v1) - (#v2)) | (# & tv1 > tv2)
		/ difference in numbers maxed with those in v1
		/ that are certainly greater than those in v2. 
	i+: 1
  ]
  :tot
}
/ compute the valence of each node given edges of the form (i,j,weight)
/ We have all symmetric edges.
/ Could do this once and for all upon input.
computevalences:{[numnodes; edges]
  x: numnodes # 0
  n: edges[;0]
  indexes: ? n
  part: = n
  counts: +/' edges[part;2]
  x[indexes]: counts
  :x
}

/ compute the valence of each node given edges of the form (i,j,weight)
/ ignoring weight.
/ We have all symmetric edges.
/ Could do this once and for all upon input.
computevalencespure:{[numnodes; edges]
  n: edges[;0]
  part: = n
  :#:' part
}

badstuff:()
/ Tries to find the most similar subgraphs among adjothers to adj1.
        / adjall,: ,edgetomatrix[nopath; currentnodecount; currentedge]
/ Improvements: Compute the valences once and for all upon input.
/ Maybe eliminate all computation of adjacency graphs.
findclosest: {[minsim; origedge1; types1; label1; vals1; origedgeothers; typesothers; labelsothers; valsothers]
   times: ()
   lasttime: _t
   adj1: edgetomatrix[nopath; #types1; origedge1]
   / vals1: countrow[nopath]'adj1
   times,: ,(" Count valence: "), ($_t - lasttime)
   lasttime: _t
   i: 0
   mysize: #adj1
   simothers: (#origedgeothers) # ,0 0 / no similarity or reverse similarity
   filtsimothers: ()
   while[i < #origedgeothers
	types2: typesothers[i]
	adj2: edgetomatrix[nopath; #types2; origedgeothers[i]]
	/ vals2: countrow[nopath]'adj2
	badflag: (#types2) < #?(origedgeothers[i])[;0] 
	if[~ badflag
	 vals2: valsothers[i]
	  / x: valtypedistnew[countrowpure[nopath]'adj1; countrowpure[nopath]'adj2; types1; types2] | typedist[types1; types2]
	/ Take inverse of vals1 because greater weight means lower value
	/ and we want to avoid counting valences of G1 that are distant
	/ as much as valences of G2.
	  x: valtypedistnew[%vals1; countrowpure[nopath]'adj2; types1; types2] | typedist[types1; types2]
	  / In principle, following should have been better, but seems worse.
	  / x: valtypedistnew[%vals1; %vals2; types1; types2] | typedist[types1; types2]
	 / x: typedist[types1; types2]
	 ]
	if[ badflag
		/ bad case
   		if[i < #labelsothers; badstuff,: labelsothers[i]]
		x: #adj1
	]
	filtsimothers,: ((#adj1) - x) % (#adj1)
	i+: 1
   ]
   if[0 = # &filtsimothers > minsim;  :( (); times)]
   times,: ,(" Cheap filtering: "), ($_t - lasttime)
   lasttime: _t
   / now find the nearest neighbor
   max: |/ filtsimothers
   i: & filtsimothers = max /  those with maximum filter similarity
   / bestadj: edgetomatrix[nopath;#typesothers[i]; origedgeothers[i]]
	/ wrong because i is a set in general
   bestadj: edgetomatrix[nopath]'[#:'typesothers[i]; origedgeothers[i]]
   simothers[i]: measurereal[adj1; types1; bestadj; typesothers[i]; label1; labelsothers[i]]
		/ this bestadj is already for the #i only
   times,: ,(" Real measurement: "), ($_t - lasttime)
   lasttime: _t
   maxreal: |/simothers[;0]
   j: & filtsimothers > maxreal / possible candidates that do better than
		/ what we can get from the filtered candidates
   i2: differ[j;i] / don't redo those already done
   times,: ,(" How many left: "), ($#i2)
   bestadj2: edgetomatrix[nopath]'[#:'typesothers[i2]; origedgeothers[i2]]
   simothers[i2]: measurereal[adj1; types1; bestadj2; typesothers[i2]; label1; labelothers[i2]]
	/ these don't overlap with the first ones
   times,: ,(" Second real measurement: "), ($_t - lasttime)
   maxreal: |/simothers[;0]
   best: :[maxreal > minsim
     & simothers[;0] = maxreal / those with greatest similarity
     ()]
   times,: ,(" Best similarity value: "), ($maxreal)
   if[0 = # best; :()] / return nothing if nothing is close enough
   result: ()
   while[0 < #best
    if[0 = #labelsothers / no labels
     first: * best
     best: 1 _ best
     bestadj3:  edgetomatrix[nopath; #typesothers[first]; origedgeothers[first]]
     exec[adj1; bestadj3; types1;  typesothers[first]; `; `] 
	/ side effect is that it creates lastalign and usefulalignindexes
     result,: ,(first; maxreal; "Types of graph A:"; types1; "Edges of graph A:"; origedge1; "Types of best match to A:"; typesothers[first]; "Edges of best match to A:"; origedgeothers[first]; "Alignment from A to best match: "; lastalign[1])
    ]
    if[(0 < #labelsothers) & (givemealignment = 0) & (kmeans = 0) / labels
        first: * best
        best: 1 _ best
        bestadj3:  edgetomatrix[nopath; #typesothers[first]; origedgeothers[first]]
        exec[adj1; bestadj3; types1;  typesothers[first]; label1; labelsothers[first]] 
	result,: ,(simothers[first; 1]; maxreal; label1; labelsothers[first])
    ]
    if[(0 < #labelsothers) & ((givemealignment = 1) | (kmeans > 0)) / labels
        first: * best
        best: 1 _ best
        bestadj3: edgetomatrix[nopath;#typesothers[first];origedgeothers[first]]
        exec[adj1; bestadj3; types1;  typesothers[first]; label1; labelsothers[first]] 
	result,: ,(simothers[first;1]; maxreal; label1; labelsothers[first]; lastalign; usefulalignindexes)
    ]
   ]
   :result
}

/ Here we measure the "real" distance from G1 to several others
/ using whichever heuristic is currently in fashion
measurereal:{[adj1; types1; adjsome; typessome; lab1; otherlabels]
    i: 0
    realdist: ()
    reversedist: ()
    while[i < #adjsome
	x: exec[adj1; adjsome[i]; types1; typessome[i]; lab1; otherlabels[i]]
	realdist,: x[1] / x[0] would be only the part in the alignment
	reversedist,: x[2]
	i+: 1
    ]
    :realdist,'reversedist
}


/ SECTION: NEIGHBOR VALENCE APPROACH
/ In this strategy we look at the degree of a node
/ and the degrees of its neighbors.

/ for each node, find the valence of the node and
/ the valence of all neighboring nodes
/ THIS SHOULD CHANGE FOR EXTENDED NEIGHBOR ??
/ FIRST THING TO DO IS SIMPLY TO COUNT THE NUMBER OF EACH POSSIBLE TYPE
/ AMONG THE NEIGHBORS. THEN THE FUNCTION vallessthanequal ALSO CHANGES.
/ MAYBE PUT THIS TYPE INFO IN AS A FINAL LIST ELEMENT.
calcneighvalenceold:{[nopath; adj]
  val: countrow[nopath]'adj / total count of edges
  size: #adj
  i: 0
  allval: ()
  while[i < size
	v: adj[i]
	neigh:  & (0 < v) / where the neighbors are.
  	/ neighval: val[neigh]
  	neighval: val[neigh] * v[neigh] / weight by value of edge to neigh
	neighval@: > neighval
	/ allval,: ,(val[i]; neighval)
	allval,: ,(val[i], neighval)
	i+: 1
  ]
  :allval
}

/ Return for each node the valence, the valence of the neighbors (properly
/ weighted) and the types of the neighbors.
/ Maybe later can get the types of the neighbors' neighbors.
calcneighvalenceold:{[nopath; adj; types]
  val: countrow[nopath]'adj / total count of edges
  size: #adj
  i: 0
  allval: ()
  while[i < size
	v: adj[i]
	neigh:  & (0 < v) / where the neighbors are.
  	/ neighval: val[neigh]
  	neighval: val[neigh] * v[neigh] / weight by value of edge to neigh
	typesval: types[neigh]
	jj: > neighval
	neighval@: jj
	typesval@: jj
	/ allval,: ,(val[i]; neighval)
	allval,: ,(val[i], neighval, ,typesval)
	i+: 1
  ]
  :allval
}

/ Return for each node the valence and the valence of the neighbors.
/ Check types and weights later.
calcneighvalence:{[nopath; adj; types]
  val: countrowpure[nopath]'adj / total count of edges
  size: #adj
  i: 0
  allval: ()
  while[i < size
	v: adj[i]
	neigh:  & (0 < v) / where the neighbors are.
  	/ neighval: val[neigh]
	typesval: types[neigh]
	jj: > typesval
	typesval@: jj
  	neighval: +/val[neigh] / weight by value of edge to neigh
	allval,: ,(val[i], neighval, ,typesval)
	i+: 1
  ]
  :allval
}

/ for each node, find the valence of the node and
/ the valence of all neighboring nodes
/ If the distance > 1, then the strength is less than 1 (the inverse)
calcneighvalencecountold:{[nopath; adj]
  val: countrow[nopath]'adj
  size: #adj
  i: 0
  allval: ()
  while[i < size
	v: 1 % adj[i] / inverse of distance
	neigh:  & (0 < v)
  	neighval: val[neigh] * v[neigh]
	allval,: ,(val[i]; +/neighval)
	i+: 1
  ]
  :allval
}

calcneighvalencecount:{[nopath; adj; types]
  val: countrow[nopath]'adj
  size: #adj
  i: 0
  allval: ()
  while[i < size
	v: 1 % adj[i] / inverse of distance
	neigh:  & (0 < v)
  	neighval: val[neigh] * v[neigh]
	typesval: types[neigh]
	allval,: ,(val[i]; +/neighval; ,typesval)
	i+: 1
  ]
  :allval
}

/ find the type consistent match between extras
findbesttypematch:{[t1; t2; ex1; ex2]
	out: ()
	tx1: t1[ex1]
	tx2: t2[ex2]
	i: 0
	while[(i < #tx1) & (0 < #tx2) 
		/ check the second to make sure there is data
		mytype: tx1[i]
		jj: tx2 ? mytype
		if[jj < #tx2
			out,: ,(ex1[i]; ex2[jj])
			ex2: ex2 _di jj
			tx2: tx2 _di jj
		]
		i+: 1
	]
	:out
}

/ find alignment based on valences of the node and its neighbors
/ and use typing.
/ If G1 is a subgraph of G2, then in the best mapping,
/ n1 of G1 should map to a node n2 in G2 such that 
/ n2 has at least as many neighbors and each
/ neighbor has at least as big a degree.
/ That is the freedom we have.
/ The _bin function here is not quite enough, since
/ if n2 has more nodes say, that is enough to cause
/ valvect1[n1] to be less than valvect2[n2].
/ What we really want is that the deg(n1) <= deg(n2)
/ and degrees of neighbors in descending order, denoted
/ neighdeg, has the property: neighdeg(n1) <= neighdeg(n2).
/ That's what we check
findalignvalall:{[origvalvect1; origvalvect2; origtypes1; origtypes2; adj1;adj2]
  valvect1: origvalvect1
  valvect2: origvalvect2
  types1: origtypes1
  types2: origtypes2
  neighs1: &:' adj1 > 0 / forget self edges
  neighs2: &:' adj2 > 0 / forget self edges
  / graph 1 from biggest to smallest, because biggest matters more
  node1: !#valvect1
  i: > valvect1
  / i: < valvect1 / no, this is bad
  node1@: i
  valvect1@: i
  types1@: i
  node1bac: node1,() / may need this later
  node2: !#valvect2
  / graph 2 from smallest to biggest, because we want binary search
  i: < valvect2
  node2@: i / we will remove nodes as they are aligned.
  types2@: i  / ditto for types
  node2bac: node2,() / need this later, already permuted
  valvect2@: i
  extra1: ()
  j: 0
  align: ()
  firstfield1: valvect1[;0]
  firstfield2: valvect2[;0]
  while[j < #node1
	/ k: valvect2 _bin v1
	firstk: firstfield2 _bin firstfield1[j]
	k: firstk
	v1: valvect1[j]
	found: 0
	/ align n = node1[j] in g1 to n' in g2 if
	/ type(n) = type(n') and val(n) <= val(n')
	/ and totalval(neighs(n)) <= totalval(neighs(n'))
	/ and types(neighs(n)) is subbag of types(neighs(n'))
	while[(found=0) & (k < #valvect2)
		v2: valvect2[k] / k is order in which we have rearranged
			/ the nodes according to their valvect2 values
		typeeq: types1[j] = types2[k] / types have been reordered too
		:[typeeq & vallessthanequal[v1; v2]; found: 1; k+: 1]
	]
	/ if[k = #valvect2; !-3] / shouldn't happen if true subgraph
	/ No. This can happen because we could have n1 from G1
	/ with 4 1 1 1 1
	/ and n1' with 3 2 2 2
	/ We will encounter n1 first because of the lexicographic order.
	/ and in G2 one has (in order) 
	/ 4 2 2 2 2
	/ and 4 3 1 1 1.
	/ So, n1 will match 4 2 2 2 2
	/ and n1' will be left high and dry.
	/ 
	/ no good, so start over
	/ align n = node1[j] in g1 to n' in g2 if
	/ type(n) = type(n') and val(n) <= val(n')
	/ and types(neighs(n)) is subbag of types(neighs(n'))
	if[found = 0
		k: firstk
		while[(found=0) & (k < #valvect2)
        		v2: valvect2[k]
			typeeq: types1[j] = types2[k]
        		:[typeeq&cheapvallessthanequal[v1; v2]; found: 1; k+: 1]
		]
	]
	 / no good, so start over. Try to align n = node1[j] in g1 to a node
	/ n' in g2 such that a neighbor m of n in g1 already is aligned
	/ with a neighbor m' of n' in g2.
	if[ (found=0) & (0 < #align)
		n1: neighs1[node1[j]]
		ii: intersectleftindexes[align[;0]; n1]
			/ do we have any neighbors in graph 1
		if[0 < #ii
		  goodneighs2: align[ii;1] / nodes that would be good 
			/ neighbors of an alignment target here
		  k: (#valvect2) - 1
		  while[(found=0) & (k > (-1))
			n2: neighs2[node2[k]]
			typeeq: types1[j] = types2[k]
			if[typeeq
				if[hasintersect[n2; goodneighs2]
					found: 1
					k+: 1
				]
			]
        		k-: 1 / if we do +1 and then -1, we get this value
		  ]
		]
	]
	/ no good, so start over
	/ align n to n' if n and n' have a neighbor with the same type
	/ Initial 0 & means not helpful.
	if[(found=0)
		k: (#valvect2) - 1
		while[(found=0) & (k > (-1))
        		v2: valvect2[k]
			typeeq: types1[j] = types2[k]
        		:[typeeq&reallycheapintersect[v1; v2]; found: 1; k-: 1]
		]
	]
	/ align n to n' if n and n' have same type
	/ Initial 0 & means not helpful. ?? Not sure whether helpful
	if[(found=0)
		k: 0
		while[(found=0) & (k < #valvect2)
			typeeq: types1[j] = types2[k]
        		:[typeeq; found: 1; k+: 1]
		]
		if[k = #valvect2; extra1,: node1[j]]
   	]
	if[found
		align,: ,(node1[j]; node2[k])
		valvect2: valvect2 _di k
		node2: node2 _di k
		types2: types2 _di k / remove these from being types
	]
	j+: 1
  ]
  / May want to drop these for subgraph-to-subgraph isomorphism.
  if[0 < (#extra1) 
	 / put extra nodes in the running, for the sake of hillclimbing
 	if[0 = #align
	   extra2: |node2bac
	]
 	if[0 < #align
	   extra2: differ[|node2bac; align[;1]] / reverse because want 
		/ nodes with largest vectors and differ preserves order.
	]
        if[0 < #extra2
	  / :[(#extra1) < (#extra2)
		/ extra2@: !#extra1
		/ extra1@: !#extra2]
	  / align,: extra1 ,' extra2
	  align,: findbesttypematch[origtypes1; origtypes2; extra1; extra2]
 	]
	extra1: differ[node1bac;align[;0]] / nothing to align these with
	if[0 < #extra1 / add those that were never matched just for
		/ the sake of hill climbing. May 28, 2000.
	   align,: extra1 ,\: (-8)
	]
  ]
  :align
}

/ attempt to swap the last few members of align with other members
/ if the degree thing can work
/ Also types should work.
attemptswap:{[origvalvect1; origvalvect2; align; numextra;origtypes1;origtypes2]
  goodalign: align[!((#align)-numextra)]
  i: (#align) - numextra
  while[(i < #align) & (0 < #goodalign)
	badalign: align[i]
	v1: origvalvect1[badalign[0]]
	v2: origvalvect2[badalign[1]]
		/ first look for type equivalence
	j1: origtypes1[badalign[0]] =/: origtypes2[goodalign[;1]]
  	j1&:  vallessthanequal[v1]'origvalvect2[goodalign[;1]]
	j2: & j1 & vallessthanequal[;v2]'origvalvect1[goodalign[;0]]
	if[0 < #j2
		x: align[i;0]
		align[i;0]: align[*j2;0]
		align[*j2;0]: x
	]
	i+: 1
  ]
  :align
}

/ vallessthanequal sees whether a vector of valence of n1 followed by 
/ valences of neighbors of n1 (neighdeg(n1)) <= val of n2, neighdeg(n2)
/ These are both vectors v1 and v2.
/ That means that v1 is no longer than v2 and that for all i in 0..|v1|-1
/ v1[i] <= v2[i].
/ It is possible that both v1 <= v2 and v2 <= v1 are false,
/ e.g. 4 1 1 1 1 and 3 2 2 2. So, the order is partial.
/ MAKE THE LAST TERM BE THE TYPES WE GET AND COMPARE ON THAT BASIS. 
/ MAYBE ALSO MAKE THIS DEPEND ON VALENCE OF FURTHER NEIGHBORS. CHANGE.
vallessthanequalold:{[v1; v2]
  if[(#v1) > (#v2); :0]
  if[(#v1) < (#v2)
	v2@: !#v1
  ]
  :&/ ~ v1 > v2
}

vallessthanequal:{[origv1; origv2]
  types1: *|origv1
  types2: *|origv2
  v1: (-1) _ origv1
  v2: (-1) _ origv2
  if[(#v1) > (#v2); :0]
  if[(#v1) < (#v2)
	v2@: !#v1
  ]
  x:(&/ ~ v1 > v2)  / dominate on valences
  if[x / if dominate on valences, check the types of
	/ the neighbors.
	x: subbag[types1; types2]
  ]
  :x
}

/ don't look at valences, just at types
/ For a relaxed test on the neighbors.
/ So will allow this pairing if types of v1 are a subset of types of v2.
reallycheapintersect:{[origv1; origv2]
  types1: *|origv1
  types2: *|origv2
  :hasintersect[types1; types2]
}

/ don't look at valences, just at types
/ For a relaxed test on the neighbors.
/ So will allow this pairing if types of v1 are a subset of types of v2.
cheapvallessthanequal:{[origv1; origv2]
  types1: *|origv1
  types2: *|origv2
  :subbag[types1; types2]
}

/ SECTION: ULLMANN Algorithm

/ given the adjacency matrices adj1 and adj2
/ the node value from G1 i
/ the node values from G2 jall (possibly more than one)
/ and currentaligns,
/ find the best of the jall nodes based on adjacency
findbestnext:{[adj1; adj2; i; jall; currentaligns]
  ineigh: & adj1[i] > 0
  mm: ()
  if[0 < #currentaligns
    mm: intersectleftindexes[currentaligns[;0]; ineigh] / are any neighbors
	/ of i involved
  ]
  if[0 = #mm  / nobody in the alignment is adjacent so try any neighbor
	/ try a neighbor with the same number of neighbors
	counts: +/' 0 < adj2[jall]
	kk: & counts = #ineigh
	if[0 < #kk
		kk1: * 1 _draw #kk
		kk: kk[kk1]
		:jall[kk]
	]
	:jall[1 _draw #jall] / if unknown then take a random chance
  ]
  / so mm is non-empty
  / Therefore i has neighbors who are already aligned.
  / We now want to see which of the jall have neighbors who
  / are aligned with a neighbor of i.
  / If any then take one.
  / To do: improve so we take only those that are really good, i.e.
  / they have a lot of other ancestors and descendants
  ibestneigh: currentaligns[mm;0] / neighbors of i in the alignment
  jbestneigh: currentaligns[mm;1] / where neighbors of i align
  neighjall: &:' adj2[jall] > 0 / neighbors of jall
		/ neighbors of jall
  poss: jall[& hasintersect[jbestneigh]'neighjall]
  if[0 < #poss;  :poss[* 1 _draw #poss]] / if found one
  :jall[1 _draw #jall] / if unknown then take a random chance
}

/ thin down the matrix
thindown:{[currentaligns; adj1; adj2; rows; mat]
   if[0 < #currentaligns
   	rows: differ[rows; currentaligns[;0]]
   ]
   if[0 = #mat; :mat]
   i: 0
   while[i < #rows
	j: & mat[rows[i]] = 1 / position where rows[i] has a 1
	if[0 < #j
	 if[1 < #j
	  / kk: * 1 _draw (#j) 
	  kk: * findbestnext[adj1; adj2; rows[i]; j; currentaligns]
	 ]
	 if[1 = #j; kk: j[0]]
	 mat[;kk]: 0
	 mat[rows[i];j]: 0
	 mat[rows[i];kk]: 1 / have only one position of the row set to 1
	 currentaligns,: ,(rows[i];kk)
	]
	i+: 1
   ]
   :mat
} 

/ Form a pair of alignments for rows having one element
/ Whose columns have one element.
formalignone:{[mapmatrix]
	counts: +/' mapmatrix
	i: & counts = 1 / ones with one target
	goodjs: mapmatrix[i] ?\: 1
	kk: & 1 = +/' + mapmatrix[;goodjs] / all columns with one item
	goodjs@: kk
	out: i[kk] ,' goodjs
	:out
}

/ Form a pair of alignments given a mapmatrix
/ or a partial one
/ Put out a list of these
formalign:{[mapmatrix]
	counts: +/' mapmatrix
	i: & counts = 1 / ones with one target
	out: i ,' (mapmatrix[i] ?\: 1)
	i2: & counts > 1 / ones with multiple targets
	out,: i2 ,' (mapmatrix[i2] ?\: 1)
	if[(#i2) > 0; !-4] / should have been eliminated
	i3: & counts < 1
	out,: i3 ,\: (-8)
	:,out
}

/ Given g1 whose n1 nodes are the rows and g2 whose n2 nodes
/ are the columns, we have a graph that says that (i,j) is 1 if
/ n1[i] has a degree that is smaller than n2[j].
/ What we want is a way to map the n1 nodes into n2 nodes as best as possible.
/ Clearly, if there is only one 1 in the n1[i] row or only one 1 
/ in the n2[j] column, we can get some reductions.
/ So, what we do is impose either row constraints or column constraints
/ or both to get initial guesses and then we go on from there.
ullimprove:{[adj1; adj2; ullmann]
  numrows: #ullmann
  numcols: #ullmann[0]
  flag: 1
  choicecount: 2
  final: ()
  while[(flag) & (choicecount < 2 + (numrows | numcols))
     flag: 1
     if[~ numrows > numcols
	/ numrows <= numcols so we can hope for G1 to be a subgraph of G2.
	/ Therefore if there are any rows with only one 1, then we take that.
	/ The first time through choicecount > 1.
	counts: +/' ullmann 
	counts1: counts
	i: & choicecount > counts
	/ i: & (choicecount > counts) & (counts > 1)
	if[0 < #i
		currentaligns: :[choicecount > 2; formalignone[ullmann]; ()]
		ullmann: thindown[currentaligns; adj1; adj2; i; ullmann]
		flag:1
	]
     ]
     if[~ numcols > numrows
	/ numrows <= numcols so we can hope for G1 to be a subgraph of G2.
	/ Therefore if there are any rows with only one 1, then we take that.
	tullmann: + ullmann
	counts: +/' tullmann 
	i: & choicecount > counts
	/ i: & (choicecount > counts) & (counts > 1)
	if[0 < #i
		currentaligns: :[choicecount > 2; formalignone[tullmann]; ()]
		ullmann: + thindown[currentaligns; adj2; adj1; i; tullmann]
			/ believe we need only to reverse the adjacency
			/ matrices in the call.
		flag:1
	]
     ]
     if[(0 = choicecount ! 10) 
	/ if[((0 = +/ (choicecount-1) < counts)&(0 = +/(choicecount-1) < counts1)) | (1 = |/ +/' ullmann)
	if[(1 = |/ +/' ullmann)
		flag: 0
	]
     ]
     choicecount+: 1
    ] / while on flag and choicecount
	/ to eliminate cases where the rows and columns
	/ have no other choices.
  / Now we try heuristics in which we go from node having few matches
  / to those having many.
  :ullmann
}

/ SECTION: SINGLE GRAPH FUNCTIONS FOR GEOMETRIC HASHING

/ finddist takes an adjacency matrix and tries to find the allpairs
/ shortest path.
/ adj is 0 for diagonal, 1 for edge and infinite (bigger than size)
/ for all else.
finddist:{[nopath; adj]
  m: adj
  newm: m distdot[nopath]\:/: +m
  while[~ newm ~ m
	m: newm
  	newm: m distdot[nopath]\:/: +m
  ]
  :_ newm + 0.5 / do this to make distances be integral, 
	/ for comparison tolerance
}

/ dot product in which we take the ith row and the jth column and we
/ get all ways to get (i,j) distances
distdot:{[nopath; v1; v2]
  i: & (v1 > nopath) & (v2 > nopath)
  if[0 = #i; :nopath]
  :&/ v1[i]+v2[i]
}

/ findnumpath takes all paths between i and j whose distance is below nopath
/ and involving no self loops
findnumpath:{[nopath; m]
  :m countpath[nopath]\:/: +m
}

/ countpath is a dot product in which we just see whether both
/ parts are less than nopath and greater than self-loops
/ and then count the paths
countpath:{[nopath; v1; v2]
   x:#& (0 < v1) & (0 < v2)
   :x
}

/ findavgdist takes average minimum distance
/ between i and j whose distance is below nopath
/ and involving no self loops
findavgdist:{[nopath; m]
  :m calcavg[nopath]\:/: +m
}

/ calcavg is a dot product in which we just see whether both
/ parts are less than nopath and no self loops and then we calculate
calcavg:{[nopath; v1; v2]
   i:& (0 < v1) & (0 < v2)
   if[0 = #i; :nopath]
   :avg[v1[i] + v2[i]]
}


/ matrixtopair -- given a pair of nodes, p calculate the matrix values
/ of all nodes to each member of the pair
/  This could be the distance, but it could be other stuff too.
matrixtopair:{[m; pair]
  n1: pair[0]
  n2: pair[1]
  :(m[n1]; m[n2])
}

/ matrixtopairavg -- given a pair of nodes, p calculate the 
/ average of the matrix values below nopath
/ of all nodes to each member of the pair
matrixtopairavg:{[nopath; m; pair]
  n1: pair[0]
  n2: pair[1]
  :(nopathavg[nopath; m[n1]]; nopathavg[nopath; m[n2]])
}

nopathavg:{[nopath;v1]
  i: & v1 > nopath
  if[0 = #i; :nopath]
  :avg[v1[i]]
}

/ choosepairs -- takes a matrix and a limit value and finds all pairs
/ below the limit
choosepairs:{[lowerlimit; upperlimit; m]
  pairs: ()
  i: 0
  while[i < #m
	/ x takes all those where m[i] <= upperlimit and lowerlimit <= m[i]
	x: & ( ~ m[i] > upperlimit) & (~ lowerlimit > m[i])
	pairs,: i ,/: x / this might give duplicate values since we could 
	 	/ have 3,13 and 13, 3
	i+: 1
  ]
  :pairs
} 



/ given an adjacency matrix, for each i, j
/ calculate distance between i and j
/ calculate number of intermediate nodes through which i can reach j
/ end for
/ The trouble with the intermediate node calculation is that
/ if one graph is a supergraph of the other, it will have too many
/ intermediate nodes.
/ Something like average distance would be better, but it's not
/ an integer.
build:{[nopath; lower; upper; adj]
  dists: finddist[nopath; adj]
  / counts: findnumpath[nopath; dists] / how many paths from i to j
  counts: findavgdist[nopath; dists]
  / note that all of these routines are independent of
  / the meaning of dists and counts. They don't have to be distances.
  p: choosepairs[lower; upper; dists]
  if[maxnumpairs < #p
	i: maxnumpairs _draw (- #p)
	p@:i
  ]
  distpair: matrixtopair[dists]'p
  inversedistpair: inverse[p; distpair]
  countpair: matrixtopairavg[nopath;counts]'p
  / inversecountpair: inverse[p; countpair] / don't need inverse.
	/ This is done after distance filtering.
  :(p; inversedistpair; countpair; distpair)
}

/ given a vector of pairs and a vector of distances to those pairs
/ (here ``distances'' can take on many meanings, e.g. can count paths)
/ find the inverse, i.e. a vector of distance pairs and a vector
/ of vectors of pair indexes.
/ So for each distance pair we have the pair indexes that this
/ maps to.
/ We sort them.
inverse:{[pairs; disttopair]
   out: ()
   i: 0
   while[i < #pairs
	d: + disttopair[i]
	out,: (,:'d) ,\: i
	i+: 1
   ]
   part: = out[;0] / partition based on distance pairs
   uniqdists: ? out[;0] / take those distance pairs
   pairids: out[part;1]
   i: < uniqdists
   x: (uniqdists[i]; pairids[i])
   :x
}


/ SECTION: COMPARING GRAPHS FOR GEOMETRIC HASHING

/ Each triple has a vector of pairs as the first element
/ second element is a vector of distances and a vector of vectors
/ of pairids
/ (Each element of the vector of vectors is a vector of pairids
/ associated with a particular distance.)
/ third element is a vector of distance-equivalences and a vector
/ of vectors of pairids.
/ Algorithm: for each distance in the second element of triple1
/ take the same distance for triple2 and count the number
/ of common votes for each pairid.
/ Spit out the pairids with their votes.
/ Returns the most promising indexes pairs of pairs.
/ For example, it might return (k1 k2) which means that pair k1 from triple1
/ and pair k2 from triple2 should match.
compare:{[triple1; triple2]
  pairs1: triple1[0] / pairs of reference nodes
  dists1: triple1[1] / dists to those reference nodes and  pairids having
	/ those distances.
  pairs2: triple2[0]
  dists2: triple2[1]
  p: intersectboth[(dists1[0]); (dists2[0])]
	/ index pairs of common distances
  votes: ,/processlists[dists1[1]; dists2[1]]'p 
	/ votes for pairids with common distances
	/ e.g. if pair id i and j both have a distance two times,
	/ then there should be two votes. 
  part:  = votes[;0] / partition based on pair id pairs (one from triple1
	/ and one from triple2
  counts: +/' votes[part;1] / sum of votes for each pair id pair
  minpairs: ? votes[;0] / In the following we give each pair and the count
  / x: (,:' pairs1[minpairs[;0]]),' (,:' pairs2[minpairs[;1]]),'counts
  x: minpairs,'counts
  max: |/ x[;2] / take the max count, giving the max number of matches
	/ for one pair compared to another.
  i: & x[;2] = max
  y: x[i]
  / Now work in the average distances to try to refine this matching
  avgdists1: triple1[2]
  avgdists2: triple2[2]
  a1: avgdists1[y[;0]] / average distances to pair from triple1
  a2: avgdists2[y[;1]] / average distances to pair from triple2
  diffs: _abs' avg'[a1-a2]
  i: & diffs = &/diffs / minimum difference
  y@: i
  x: y[;0 1]
  :x
}

/ indexes is a pair whose first element is for refpairs1
/ and whose second element is for refpairs2
/ refpairs1 is a vector of vectors of reference pair ids
/ Therefore an index gives a single vector of reference pair ids.
/ So, we will have two reference pair vectors.
/ Let's say we have (1 1 2 3) and (1 2 4 4 5)
/ These represent one vote for (1 1), one vote for (1 2),
/ two votes for (1 4) etc. 
/ In general (i, j) gets k votes if the 
/ minimum of the number of is in the first list
/ and the number js in the second list is k.
/ Returns a pair of reference pair ids and a count.
processlists:{[refpairs1; refpairs2; indexes]
  rp1: refpairs1[indexes[0]]
  rp2: refpairs2[indexes[1]]
  cpart1: #:' = rp1
  cpart2: #:' = rp2
  x: ,/ (?rp1) ,/:\: (?rp2)
  y: ,/ cpart1 ,/:\: cpart2 / cross products of counts
  z: y[;0] & y[;1]
  :(,:'x),'z
}

/ findmapping tries to find an isomorphism based on a best pair mapping
/ bestpairs are the pairs of references that do the best job
/ of matching distances, e.g. ((i;j);(i';j'))
/ pairmap1 are the distances to one pair, pairmap2 to another pair.
findmappingOLD:{[nopath; adj1; adj2; bestpairs; pairmap1; pairmap2; types1; types2]
  x: findalign[nopath; adj1; adj2; pairmap1; pairmap2; types1;types2]'bestpairs 
	/ look at all alignments
  part: = x
  uniqaligns: ?x
  counts: #:' part / find one that happens the most often
  i: > counts
  best: uniqaligns[*i]
  :best
}

/ findmappingall  is like findmapping except that it returns all alignments
findmappingall:{[nopath; adj1; adj2; bestpairs; pairmap1; pairmap2; types1; types2]
  :?findalign[nopath; adj1; adj2; pairmap1; pairmap2; types1; types2]'bestpairs 
	/ look at all alignments
}


/ findalign finds a good alignment for a given pair of reference pairs.
/ One version now deprecated:
/ In finding an alignment for a node, strive to find a match with the
/ same degree or perhaps larger.
/ It uses the bestpair heuristic but then orders the matches by degree.
findalign:{[nopath; adj1; adj2; pairmap1; pairmap2;  types1; types2; bestpair]
  p1: + pairmap1[bestpair[0]]
  p2: + pairmap2[bestpair[1]]
  part1: = p1,' types1[!#p1] / partition by distance to ref set and type
  part2: = p2,' types2[!#p2]
  p1uniq: ? p1,' types1[!#p1]
  p2uniq: ? p2,' types2[!#p2]
  x: intersectboth[p1uniq; p2uniq] / just get indexes in part1 and part2
  i: 0
  align: ()
  extra1: ()
  extra2: ()
  while[i < #x
	next: x[i]
	v1: part1[next[0]] / these are node ids
	v2: part2[next[1]]
	count1: countrow[nopath]'adj1[v1]
	count2: countrow[nopath]'adj2[v2]
	j1: < count1
	j2: < count2
	v1@: j1
	v2@: j2
	if[(#v1) > (#v2) / we remove the lower ones and put them in extra1. 
		/ That seems to be the best strategy.
		extra1,: v1[!(#v1) - (#v2)]
		v1: ((#v1) - (#v2)) _ v1
		/ now try removing the upper ones
		/ extra1,: v1[(#v2) + !(#v1) - (#v2)]
		/ v1@: !(#v2)
	]
	if[(#v2) > (#v1)
		extra2,: v2[!(#v2) - (#v1)]
		v2: ((#v2) - (#v1)) _ v2
		/ now try removing the upper ones
		/ extra2,: v2[(#v1) + !(#v2) - (#v1)]
		/ v2@: !(#v1)
	]
	align,: v1 ,' v2
	i+: 1
  ]
  if[(0 < #extra1) & (0 < #extra2)
    / first partition based on types then try for the best
    part1: = types1[extra1] / indexes within extra1
    part1: extra1[part1] / real nodeids
    part2: = types2[extra2]
    part2: extra2[part2] / real nodeids
    tuniq1: ? types1[extra1]
    tuniq2: ? types2[extra2]
    x: intersectboth[tuniq1; tuniq2] / just get indexes in part1 and part2
		/ these are the same types
    i: 0
    while[i < #x
	next: x[i]
	e1: part1[next[0]]
	e2: part2[next[1]]
  	count1: countrow[nopath]'adj1[e1]
  	count2: countrow[nopath]'adj2[e2]
  	j1: < count1
  	j2: < count2
	e1@: j1
	e2@: j2
	if[(#e1) < (#e2); e2@: !#e1]
	if[(#e2) < (#e1); e1@: !#e2]
	align,: e1 ,' e2
	i+: 1
    ]
  ]
  :align
}

/ mapbydegree tries to find an alignment by sorting the two graphs
/ by degree and then aligning by degree
/ If there are several possible ones, it produces all
mapbydegree:{[nopath; adj1; adj2; types1; types2]
  edgecount1: countrow[nopath]'adj1
  edgecount2: countrow[nopath]'adj2
  part1: = edgecount1,' types1[!#edgecount1]
  part2: = edgecount2,' types2[!#edgecount2]
  uniq1: ? edgecount1,' types1[!#edgecount1]
  uniq2: ? edgecount2,' types2[!#edgecount2]
  i1: < uniq1
  i2: < uniq2
  part1@: i1
  part2@: i2
  allalign: ()
  do[maxnumpairs
   allalign,: ,genalign[part1; part2]
  ]
  :allalign
}

/ genalign takes two sets of partitions and permutes each partition
/ randomly and then lengthens out
genalign:{[part1; part2]
  part1: rearrange'part1
  part2: rearrange'part2
  x1: ,/part1
  x2: ,/part2
  if[(#x1) < (#x2); x2@: !#x1]
  :((x1) ,' (x2))
}

/ rearrange a vector
rearrange:{[v]
  : v[(#v) _draw - #v]
}

/ CHECK THE ALIGNMENT


/ checkalign sees how good an alignment is by seeing which percentage
/ of the aligned graphs in fact are consistent
/ Question: should this be of all possible edges or just of 
/ those that I have found.
/ This routine returns both numbers.
checkalign:{[nopath; adj1; types1; adj2; types2; lab1; lab2; align]
  jj: & align[;1] > (-1) / eliminate those extra ones that are bound to nothing
  align@: jj
  if[~ (#align[;0]) = (#?align[;0]); !-1]
  if[~ (#align[;1]) = (#?align[;1]); !-2] / bad alignment
  / new way to evaluate alignment, seems like cheating
  / same: intersect[align; realalign]
  / x: ((#same)%(#realalign); (#same)%(#align))
  / :x
  neigh1: findneighborsnodistance[nopath; align[;0]; adj1] / all edges in adj1
	/ for nodes that participate in alignment.
  neigh2: findneighborsnodistance[nopath; align[;1]; adj2]
  neigh1aligned: replace[align;neigh1] / if {i,j} is an edge in neigh1
	/ then {m[i],m[j]} is an edge in neigh1aligned
	/ where i, m[i] is a row in align.
  y: intersectboth[neigh1aligned; neigh2] / indexes of edge matches
	/ disregards edge strengths (distances)
  if[0 = #y; :(0;0; 0; (); #neigh1; neigh1; neigh2;())]
  y@: & matchtype[neigh1; types1; neigh2; types2;y]
  samecount: 0
  if[0 < #y; samecount: evalmatch[neigh1; adj1; neigh2; adj2; y; lab1; lab2]]
	/ Look at each element of alignment and 
	/ evaluate the weights of the edges to decide on strength.
  / same: intersect[neigh2; neigh1aligned]
  / allneighcount: (countedge[nopath; adj1]) | (countedge[nopath;adj2])
  allneighcount1: (countedge[nopath; adj1]) 
  allneighcount2: (countedge[nopath; adj2]) 
  jackextras: (samecount) % allneighcount2 / over edges of g2
  usefulindexes: intersectleftindexes[align[;0]; ?,/neigh1[y[;0]]]
	/ y[;0] are indexes of neigh1aligned and therefore of neigh1
	/ that find matches. The g1 nodes of those edges are
	/ ,/neigh1[y[;0]]
  x: ((samecount)%#neigh1; ((samecount) % allneighcount1); jackextras; neigh1aligned; allneighcount1; neigh1; neigh2; usefulindexes )
 	/ First number is proportion correct for subgraph in alignment
	/ where nodes outside the subgraph can count for errors.
	/ Second number is the proportion of edges correctly covered
	/ for first graph.
  :x
}

/ hillcheckalign sees how good an alignment is by seeing which percentage
/ of the aligned graphs in fact are consistent
/ Question: should this be of all possible edges or just of 
/ those that I have found.
/ It's optimized for hill-climbing where we can avoid certain checks.
hillcheckalign:{[nopath; adj1; types1; adj2; types2; align; allneighcount1; allneighcount2; lab1; lab2]
  jj: & align[;1] > (-1) / eliminate those extra ones that are bound to nothing
  align@: jj
  neigh1: findneighborsnodistance[nopath; align[;0]; adj1] / all edges in adj1
	/ for nodes that participate in alignment.
  neigh2: findneighborsnodistance[nopath; align[;1]; adj2]
	/ all edges in g2 that participate in the alignment
  neigh1aligned: replace[align;neigh1] / if {i,j} is an edge in neigh1
	/ then {m[i],m[j]} is an edge in neigh1aligned
	/ where i, m[i] is a row in align.
  y: intersectboth[neigh1aligned; neigh2] / indexes of edge matches
	/ disregards edge strengths (distances)
  if[0 = #y; :(0;0; 0; (); #neigh1; neigh1; neigh2; ())]
  y@: & matchtype[neigh1; types1; neigh2; types2;y]
  samecount: 0
  if[0 < #y; samecount: evalmatch[neigh1; adj1; neigh2; adj2; y; lab1; lab2]]
	/ Look at each element of alignment and 
	/ evaluate the weights of the edges to decide on strength.
  / same: intersect[neigh2; neigh1aligned]
  jackextras: (samecount) % allneighcount2 / over edges of g2
  usefulindexes: intersectleftindexes[align[;0]; ?,/neigh1[y[;0]]]
	/ y[;0] are indexes of neigh1aligned and therefore of neigh1
	/ that find matches. The g1 nodes of those edges are
	/ ,/neigh1[y[;0]]
  x: ((samecount)%#neigh1; ((samecount) % allneighcount1); jackextras; neigh1aligned; allneighcount1; neigh1; neigh2; usefulindexes )
 	/ First number is proportion correct for this subgraph
	/ where nodes outside the subgraph can count for errors.
	/ Second number is the proportion of edges correctly covered
	/ for first graph.
  :x
}

/ see if edges in neigh1 typematch  corresponding edges in neigh2
/ The correspondence is determined by index match.
/ If alignment has preserved types, then this should always hold.
matchtype:{[neigh1; types1; neigh2; types2;indexmatches]
  t1: types1[neigh1[indexmatches[;0];0]] ,' types1[neigh1[indexmatches[;0];1]]
  t2: types2[neigh2[indexmatches[;1];0]] ,' types2[neigh2[indexmatches[;1];1]]
  :t1 ~' t2
}

/ Evaluate each edge match for its strength.
/ neigh1 is a bunch of pairs in adj1 e.g. {i,j}. 
/ neigh2 is a bunch of pairs in adj2 e.g. {i',j'}. 
/ indexmatches tells which pair goes with which.
/ Suppose {i,j} is aligned with {i',j'}.
/ Then we look at the distances of the two edges d1 and d2
/ and take the minimum of d1/d2 and d2/d1.
evalmatchold:{[neigh1; adj1; neigh2; adj2; indexmatches]
  e1: neigh1[indexmatches[;0]]
  e2: neigh2[indexmatches[;1]]
  / edges are now aligned
  w1: adj1[e1[;0]] @' e1[;1] / weights of e1
  w2: adj2[e2[;0]] @' e2[;1] / weights of e2
  z: +/ (w1 % w2) &' (w2 % w1)
  :z
  :_ z + 0.5 / don't need to be integral
}

evalmatch:{[neigh1; adj1; neigh2; adj2; indexmatches; lab1; lab2]
 e1: neigh1[indexmatches[;0]]
 e2: neigh2[indexmatches[;1]]
 if[multiedgeflag = 0 / normal case of a single edge
  / edges are now aligned
  w1: adj1[e1[;0]] @' e1[;1] / weights of e1
  w2: adj2[e2[;0]] @' e2[;1] / weights of e2
  z: +/ (w1 % w2) &' (w2 % w1)
  :z
  :_ z + 0.5 / don't need to be integral
 ]
 if[multiedgeflag = 1 / multiedges, so weight is a vector
  findsets:{[lab; e]
    i: realoriglabel ? lab
    edge: realorigedgeall[i]
    good: edge[;0] ,' edge[;1]
    want: e[;0] ,' e[;1]
    j: good ?/: want
    sets: 2 _' edge[j]
    :sets
  }
  sets1: findsets[lab1; e1]
  sets2: findsets[lab2; e2]
  sizes1: #:' sets1
  sizes2: #:' sets2
  intersizes: #:' intersect'[sets1; sets2]
  z: +/ (intersizes % sizes1) &' (intersizes % sizes2)
  :z
 ]
}
   
   


dotprod:{[x;y] +/x*y}

/ does exchanges of alignments to see if there is something better
/ in the neighborhood. Uses pure greed.
/ The size of the swaps goes down over the course of the algorithm.
/ At first, it is all members of a type and then it goes down.
/ For small graphs anyway, the dot product doesn't speed things up
/ and hurts the result slightly.
hillclimb:{[nopath; adj1;adj2;currentbestalign; currentbestscore;types1; types2; lab1; lab2]
 alignlist:,currentbestalign 
 / dotalignlist: dotprod[currentbestalign[;0]; currentbestalign[;1]], ()
 allneighcount1: (countedge[nopath; adj1]) 
 allneighcount2: (countedge[nopath; adj2]) 
 / alreadyseen: 0
  improve:: 0
  step: 0
  swaphist: ()
 do[maxnumpairs
  step+: 1
  size: #currentbestalign
  x: *1 _draw size
  jj: & types1[currentbestalign[;0]] = types1[currentbestalign[x;0]]
  numinperm: (_ 1 + (maxnumpairs % step)) & (#jj) / number to be permuted
	/ have a lot at the beginning and few at end
  swaphist,: numinperm
  newjj: jj[(#jj) _draw (- #jj)]
  new: currentbestalign
  new[newjj;0]: new[jj;0]
   / dotnew: dotprod[new[;0]; new[;1]]
   flag: new _in alignlist
   / if[flag; alreadyseen+: 1]
   if[~ flag
    alignlist,: ,new
    / dotalignlist,: dotnew
    zz: hillcheckalign[nopath; adj1; types1; adj2; types2; new; allneighcount1; allneighcount2; lab1; lab2]
    if[zz[1] > currentbestscore[1] / zz[1] is score over all edges
		/ of g1
		/ not just those edges of g1 in the alignment
	currentbestalign: new
	currentbestscore: zz[0 1 2] / zz[0] is score over edges in alignment
		/ zz[2] is score of edges in proper alignment over g2
	currentusefulalignindexes: zz[7] / indexes of alignment that help
	improve+: 1
    ]
   ]
 ] / maxnumpairs
 :(currentbestscore; currentbestalign; currentusefulalignindexes)
}



/ findneighbors spits out pairs of neighbors
/ based on nopath
/ A neighbor is a neighbor regardless of distance (i.e. edge
/ strength).
/ But we spit out distance anyway
findneighbors:{[nopath; goodrows; adj]
  out: ()
  i: 0
  while[i < #goodrows
	r: goodrows[i]
	j: & (0 < adj[r]) / true neighbors
	out,: (r ,/: j) ,' adj[r][j]
	i+: 1
  ]
  :out
}

/ TRANSLATION FUNCTIONS

/ findneighbors spits out pairs of neighbors
/ based on nopath
/ A neighbor is a neighbor regardless of distance (i.e. edge
/ strength).
/ But we spit out distance anyway
/ MUCH FASTER
findneighbors:{[nopath; goodrows; adj]
  findneighone:{[nopath; adj;r] 
	j: & (0 < adj[r])
	: (r ,/: j) ,' adj[r][j]
  }
  out: ,/ findneighone[nopath; adj]'goodrows
  :out
}

/ find neighbors disregarding distances
findneighborsnodistanceold:{[nopath; goodrows; adj]
  out: ()
  i: 0
  while[i < #goodrows
	r: goodrows[i]
	j: & (0 < adj[r]) / true neighbors
	out,: (r ,/: j) 
	i+: 1
  ]
  :out
}


/ find neighbors disregarding distances
/ MUCH FASTER
findneighborsnodistance:{[nopath; goodrows; adj]
  / I think we want to spit out only those edges both of whose
  / members are in the alignment
  findneighone:{[nopath; adj;goodnodes; i] 
	:i ,/: intersect[(& (0 < adj[i])); goodnodes]}
  out: ,/ findneighone[nopath; adj; goodrows]'goodrows
  / out: ,/ findneighone[nopath; adj]'!#adj / temporary: all rows
		/ this wasn't good.
  :out
}

/ findweights spits out the weight of each edge in turn
/ based on nopath.
findweights:{[nopath; goodrows; adj]
  out: ()
  i: 0
  while[i < #goodrows
	r: goodrows[i]
	j: & (0 < adj[r]) / true neighbors
	w: adj[r;j]
	out,: w
	i+: 1
  ]
  :out
}

/ countedge counts the total number of edges in a graph
/ other than self-edges or nopath edges
/ I don't know what to do with weaker edges.
/ I'm tempted to count them as less important.
/ But then we might get similarities > 1.
/ If edges of distance 2 match, that is as important as if
/ edges of distance 1 match isn't it?
/ Let's say it is.
countedge:{[nopath; adj]
  :+/countrowpure[nopath]'adj
}

countrowpure: {[nopath; v] # & (0 < v)}
countrow: {[nopath; v] +/  v[& (0 < v)]} 
	/ These edges are potentially over 1 (more distant).
	/ So, the strength is the inverse.

/ replace takes the alignment and remaps nodes into their
/ new node names.
/ We take advantage of the fact that the first column of neigh
/ is the same as the first column of align.
/ This could leave edges around that had never been mapped.
replaceold:{[align; neigh]
  i: align[;0] ?/: neigh[;1] / which of the targets of the neighbors
	/ belong to the domain of the alignment mapping
  j: & i < #align[;0] / all the good ones
  target: neigh[;1]
  target[j]: align[i[j]; 1]
  / for the neigh[;0] we know all of these must hit
  i: align[;0] ?/: neigh[;0] / all should be less than #align[;0]
  source: neigh[;0]
  source: align[i;1]
  x: source ,' target
  :x
}

/ replace takes the alignment and remaps nodes into their
/ new node names.
/ We take advantage of the fact that the first column of neigh
/ is the same as the first column of align.
/ recall that align maps from g1 to g2. neigh are edges in g1.
replace:{[align; neigh]
  if[0 = #neigh; :()]
  if[0 = #align; :()]
  i1: align[;0] ?/: neigh[;0] / align[;0] is unique
  i2: align[;0] ?/: neigh[;1] / align[;0] is source of mapping
  j: & (i1 < #align[;0]) & (i2 < #align[;0]) / good neighbor pairs
  x: align[i1[j];1] ,' align[i2[j];1]
  :x
}

/ swap the node ids in the edges based on swappair 
/ Any node matching swappair[0] should become swappair[1]
/ and vice versa.
swapnodes:{[edges; swappair]
 j: 0
 while[j < 2
  i0: & edges[;j] =  swappair[0]
  i1: & edges[;j] = swappair[1]
  edges[i0;j]: swappair[1]
  edges[i1;j]: swappair[0]
  j+: 1
 ]
 :edges
}

/ SECTION: RANDOM DATA GENERATION ROUTINES

edgeweight: 4 / for experimental purposes make this uniform

/ generate an adjacency matrix with nodecount nodes and edgecount non-self
/ edges.
randgen:{[nopath; nodecount; edgecount]
  out: (nodecount;nodecount) # nopath / no edges initially
  i: 0
  while[i < nodecount
	out[i;i]: 0
	i+: 1
  ]
  j: 0
  while[j < edgecount
	k1: * 1 _draw nodecount
	k2: * 1 _draw nodecount
	if[nopath = out[k1;k2]
		yy: * 1 _draw (_ 1 + edgeweight)
		yy|: 1
		out[k1;k2]: yy
		out[k2;k1]: yy
		j+: 1
	]
  ]
  :out
}

/ assign types to nodes
randtypes:{[nodecount; numtypes]
  :nodecount _draw numtypes
}


/ map edges into an adjacency matrix
/ If edgeweight is 1, then all the same.
/ Otherwise distances can be > 1.
edgetomatrix:{[nopath; nodecount; edges]
 / edges[2]: 1 + %edges  / if edges connote strength instead of distance ???
 out: (nodecount;nodecount) # nopath / no edges initially
 i: 0
 while[i < nodecount
       out[i;i]: 0
       i+: 1
 ]
 j: 0
 while[j < #edges
	k1: _ edges[j;0] / if nodes start at one subtract 1: _ edges[j;0] - 1
	k2: _ edges[j;1] 
	/ yy: :[2 < #edges[j]; 1 + %edges[j;2]; 1.0]
	  / ??? If DISTANCE IS STRENGTH, it IS INVERTED
	yy: :[2 < #edges[j]; edges[j;2]; 1]
	out[k1;k2]: yy
	out[k2;k1]: yy
	j+: 1
  ]
  :out
}

/ map edges into an adjacency matrix
/ put in extra edges
/ nodecount might be bigger than nodecount for another matrix
edgetomatrixextra:{[nopath; nodecount; edges; extraedges; weights]
 out: (nodecount;nodecount) # nopath / no edges initially
 i: 0
 while[i < nodecount
       out[i;i]: 0
       i+: 1
 ]
 j: 0
 while[j < #edges
	k1: edges[j;0]
	k2: edges[j;1]
	yy: weights[j]
	out[k1;k2]: yy
	out[k2;k1]: yy
	j+: 1
  ]
  j: 0
  while[j < extraedges
	k1: * 1 _draw nodecount
	k2: * 1 _draw nodecount
	if[nopath = out[k1;k2]
		yy: * 1 _draw (_ 1 + edgeweight)
		yy|: 1
		out[k1;k2]: yy
		out[k2;k1]: yy
		j+: 1
	]
  ]
  :out
}

/ permute takes a set of edges and relabels nodes in a way
/ that maintains the consistency of the edges
permute:{[edges]
  x: ?intersect[edges[;0]; edges[;1]]
  targ: x[(#x) _draw -(#x)]
  align: x ,' targ
  realalign:: align
  :replace[align; edges]
}

randomalignall:{[nodecount1; nodecount2; types1; types2]
  allalign: ()
  do[maxnumpairs
	allalign,: ,randomalign[nodecount1; nodecount2; types1; types2]
  ]
  :allalign
}

/ randomalign gives a random alignment of a set of nodes
randomalign:{[nodecount1; nodecount2; types1; types2]
   / source: !nodecount
   / target: nodecount _draw -nodecount
   / :source,'target
    align: ()
    / first partition based on types then try for the best
    extra1: !nodecount1
    extra2: !nodecount2
    part1: = types1[extra1] / indexes within extra1
    part2: = types2[extra2]
    tuniq1: ? types1[extra1]
    tuniq2: ? types2[extra2]
    x: intersectboth[tuniq1; tuniq2] / just get indexes in part1 and part2
		/ these are the same types
    i: 0
    while[i < #x
	next: x[i]
	e1: part1[next[0]]
	e2: part2[next[1]]
	if[(#e1) < (#e2); e2@: !#e1]
	if[(#e2) < (#e1); e1@: !#e2]
	align,: e1 ,' e2
	i+: 1
    ]
    :align
}

/ randomalign gives a random alignment of a set of nodes
randomalignold:{[nodecount]
   source: !nodecount
   target: nodecount _draw -nodecount
   :source,'target
}

/ HEURISTIC-DRIVEN EXECUTION OF GRAPH COMPARISON

	/ reverse comparison too.
/ Execution of the basic graph comparison. It works according to
/ the heuristic used.
exec:{[adj1; adj2; types1; types2; lab1; lab2]
if[heuristic = 5
  valvect1: countrowpure[nopath]'adj1
	/ just need the valence of each node
  valvect2: countrowpure[nopath]'adj2
  / so finds all cases where 
  / number <= vec and types are consistent
  findones:{[othernumber; othertype; mynumber; mytype] 
	:(~ mynumber > othernumber) & (mytype = othertype)} 
  ullmann: findones[valvect2; types2]'[valvect1; types1]
  ullmann2: ullimprove[adj1; adj2; ullmann]
  x: formalign[ullmann2]
]
if[heuristic = 4
  valvect1: calcneighvalence[nopath; adj1; types1] 
	/ just need the valence of each node
  valvect2: calcneighvalence[nopath; adj2; types2]
  x: ,findalignvalall[valvect1; valvect2; types1; types2; adj1; adj2] 
]
t1:: _t
if[heuristic = 3
  triple1: build[nopath; lower; upper; adj1]

  triple2: build[nopath; lower; upper; adj2]
  t1:: _t
  bestpairs: compare[triple1; triple2]
]
t2:: _t
t4:: _t

if[heuristic = 3
x: findmappingall[nopath; adj1; adj2; bestpairs; triple1[3]; triple2[3]; types1; types2]
]
if[heuristic = 1
x: mapbydegree[nopath; adj1; adj2; types1; types2]
]
if[heuristic = 0
x: randomalignall[nodecount; nodecount+extranodes; types1; types2] 
	/ for testing a random alignment
]
/ x: ,realalign
counts: checkalign[nopath; adj1; types1; adj2; types2; lab1; lab2]'x
/ i: >counts[;0] / this would work if we were counting only edges
	/ in the alignment
i: >counts[;1] / this works for proportion of properly aligned edges
	/ against all edges
zz: counts[*i][0 1 2] / best count with other fields
lastalign:: x[*i] / current best alignment
usefulalignindexes:: !#lastalign
if[(zz[1] < 1) & (hillclimbing = 1) / improve this through hillclimb
  if[0 < # x[*i] / there is some alignment to improve
  	hillclimbresult:hillclimb[nopath; adj1; adj2; x[*i]; zz; types1; types2; lab1; lab2]
  	lastalign:: hillclimbresult[1] / improved alignment
	usefulalignindexes:: hillclimbresult[2] 
		/ parts of alignment that do good
  	zz: hillclimbresult[0] / just get scores 
  ]
]
t5:: _t
lastalign@: & (-1) < lastalign[;1]
usefulalignindexes:: !#lastalign
:zz
}

/ CLUSTERING



/ given a targetset and a set of sets called the sourcesets
/ such that the union of the sourcesets equals the targetset,
/ we want the smallest subset of these sourcesets whose union
/ equals the targetset.
/ Algorithm: start with largest subset and then try to reduce
/ difference as fast as possible.
findmax:{[targetset; sourcesets]
  counts: #:'sourcesets
  out: ()
  ii: > counts
  current: sourcesets[ii[0]]
  out,: ii[0]
  extra: differ[targetset; current] / what more is needed
  while[0 < #extra
	extracount: #:'intersect[extra]'sourcesets
	mm: > extracount
	out,: *mm
	current,: sourcesets[*mm]
	current?:
  	extra: differ[targetset; current]
  ]
  :out @ < out
}


/ same as findmax but does random
findmaxrand:{[targetset; sourcesets]
  counts: #:'sourcesets
  out: ()
  ii: > counts
  nn: ii[* 1 _draw #ii]
  current: sourcesets[nn]
  out,: nn
  extra: differ[targetset; current] / what more is needed
  while[0 < #extra
	extracount: #:'intersect[extra]'sourcesets
	mm: > extracount
	out,: *mm
	current,: sourcesets[*mm]
	current?:
  	extra: differ[targetset; current]
  ]
  :out @ < out
}

spitlist:{[list] (-1) _ ,/ ($list) ,\: (" ")}

/ see if you can get kmeans clusters based on the given cutoff
/ for  these distances
getclusts:{[allels; kmeans; cutoff;pairwise]
  i: & ~ pairwise[;2] < cutoff / these are the only
	/ distances that work
  mypairs: pairwise[i]
  if[0 = #mypairs;  :(();())]
  part: = mypairs[;0]
  counts: #:' part
  counts@: > counts
  maxallowed: (kmeans) & (#counts)
  if[(maxallowed + (+/ counts[!maxallowed])) < #allels
	:(();()) / no way to cover everything
  ]
  sources: ?mypairs[;0]
  targets: mypairs[part;1]
  sims: mypairs[part;2]
  minsims: &/' sims
  ii: findmax[allels; sources,'targets]
  j: 0
  while[(j < 50) & (kmeans < #ii) / if you can't cover set with
		/ kmeans (which is an int) clusters, then try again
  	ii: findmaxrand[allels; sources,'targets]
	j+: 1
  ]
  if[kmeans < #ii; :(();())]
  sources@: ii
  targets@: ii
  sims@: ii
  x: sources,'sources,'(#sources) # 1.0
	/ we know that these are going to be our centroids
	/ Add them in to ensure that we don't have duplicate entries
  x,: (,/(#:'targets) #' sources) ,' (,/targets),' (,/sims)
	/ distances of each target to each centroid
  part: = x[;1]
  maxsim: |/'x[part;2] / maximum similarity of a target
  outindex: ()
  j: 0
  while[j < #part
	mymax: maxsim[j]
	i: (x[part[j];2]) ? mymax
	outindex,: (part[j])[i]
	j+: 1
  ]
  x@: outindex
  part: = x[;0]
  sources: ?x[;0]
  targets: x[part;1]
  dists: x[part;2]
  x: differ[allels; ?sources,,/targets] / get those not in any cluster
  sources,: x
  targets,: (#x) # ,()
  dists,: (#x) # ,1.0
  x: ` $ ("minsim ") ,/: ($&/'dists) ,' (" --- ") ,/: ($ sources) ,'  (": ") ,/: (spitlist'?:'sources,'targets)
  y: (sources; ?:'sources,'targets)
  :(x; y)
}

/ find the kmeans graphs that best match to the other graphs
/ dists are source, target, dist
/ source != target
/ all elements are in source
findbestclusters:{[kmeans; pairwise]
  allels: ?pairwise[;0]
  / changed may 15, 2007 to avoid having same element in pairwise
  ii: & (~ (pairwise[;0]) = (pairwise[;1]))
  pairwise@: ii
  / out: allels ,' allels ,' (#allels) # 1
  flag: 1
  worked: 0
  noworked: 1.2
  while[0 & (noworked - worked) > 0.1 / don't need this
  	cutoff: (worked + noworked) % 2
  	clusters: getclusts[allels; kmeans; cutoff;pairwise]
	clusters@: 0
  	if[0 < #clusters
		worked: cutoff
	]
  	if[0 = #clusters
		noworked: cutoff
	]
  ]
  x: getclusts[allels; kmeans; (worked+noworked)%2; pairwise]
  :x
}

/ MINIMAL MATCHERS



/ CONNECTED COMPONENTS

 / labelsall:: ()
 / origedgeall:: ()
 / typesall:: ()
/ find the best subgraph of the head of the cluster
/ to make it match the other graphs in the cluster with the
/ greatest possible similarity.
findbesthead:{[comps; cluster]
  first: cluster[0]
  others: cluster[1] _dv first
  firstindex: labelsall ? first
  if[0 = #others 
	xx: origedgeall[firstindex]
	:(first; ? xx[;0],xx[;1] ;typesall[firstindex]; origedgeall[firstindex])
  ]
  i: & (comps[;0] = first) & (comps[;1] _in\: others)
  mycomps: comps[i]
  part: = mycomps[;0 1]
  findmax:{[lists] 
     maxval: |/lists[;2]
     i: lists[;2] ? maxval
     :lists[i]
  }
  mycomps: findmax'mycomps[part] / get max similarity for each i,j pair
		/ i,j appear more than once because once for mapping
		/ from i to j and once for mapping in other direction
  xx: mycomps[;3] / alignments
  firstcol:{[list] list[;0]}
  yy: firstcol'xx
  alldesired: ,/multiintersect[yy]  / all the nodes we want in 
		/ the best alignments
  alldesired: ?,/yy / get union of all good alignments
  g: findminimal[alldesired;  origedgeall[firstindex]]
  g[0]@: < g[0]
  / :(first; typesall[firstindex;g[0]]; g[1]) 
	/ return label, types of nodes, edges
  :(first; g[0]; (typesall[firstindex;g[0]]); g[1]) 
	/ return label, nodes, types; edges
}

/ reducenodenum takes the node numbers in a set of edges and maps
/ them as low as possible, so if one has (1 2 2); (4 2 3), this will map to
/ (0 1 2); (2 1 3)
reducenodenum:{[edges]
  nodes: ? edges[;0],edges[;1]
  nodes@: < nodes
  source: nodes
  target: !#nodes
  edges[;0]: target[source ?/: edges[;0]]
  edges[;1]: target[source ?/: edges[;1]]
  :edges
}
  
/ take distances in format
/ (destsourcedist, sourcedestdist, source, dest, align source to dest, usefulindexes)
/ and convert to (n1, n2, distn1n2, alignn1n2)
/ The alignment chosen consists of those edges that are actually useful
/ in the alignment.
clusterexpand:{[dist]
  a: dist[4]
  a@: dist[5]
  b: |:' a
  out: ,(dist[3]; dist[2]; dist[0]; b)
  out,: ,(dist[2]; dist[3]; dist[1]; a)
  :out
}


/ This module starts with a bunch of nodes that must be preserved
/ desirednodes
/ (because they are in an alignment)
/ and tries to remove other nodes while still keeping the graph connected.
/ The function returns the smallest set of nodes it can find (and the
/ smallest set of edges) that works.
findminimal:{[desirednodes; edges]
  allnodes: ? desirednodes, edges[;0], edges[;1]
  cands: differ[allnodes; desirednodes]
  minsize: #allnodes
  bestsofar: allnodes
  testedges: edges[;0 1]
  j: 0
  while[(j < 100) & (minsize > #desirednodes)
	newcands: cands[(#cands) _draw -#cands]
	trynodes: newcands, desirednodes
	flag: 1
	i: 0
 	while[(i < (#newcands))  & flag
		lastworked: trynodes
		trynodes: 1 _ trynodes
		flag: isconnected[trynodes; testedges]
		i+: 1
	]
	if[flag; lastworked: trynodes]
	if[(#lastworked) < minsize
		minsize: #lastworked
		bestsofar: lastworked
	]
	j+: 1
  ]
  i: & (edges[;0] _in\: bestsofar) & (edges[;1] _in\: bestsofar)
  :(bestsofar; edges[i])
}


/ Is the subgraph consisting of nodes connected given the edges?
/ Algorithm: compute the transitive closure restricted to nodes in set
/ Then each node goes in the component of its first 1.
/ This makes no assumptions about the edges (e.g. it's fine
/ if there is an edge 17 3.)
isconnected:{[nodes; edges]
   mat: transitive[nodes;edges]
   i: * nodes
   x: subset[nodes; & mat[i]]  / nodes are connected to at least one node
	/ if all 1 then the graph is connected, otherwise not.
   :x
}

/ logical dot product
logicdot:{[v1;v2] |/ v1*v2}


/ find transitive closure of a subgraph consisting
/ of a set of nodes and the induced edges
/ and return a matrix.
/ An edge is removed unless both its endpoints belong to nodes.
/ We want the transitive closure of the induced subgraph.
transitive:{[nodes; edges]
  newedges: edges
  newedges,: |:' newedges
  newedges,: nodes,' nodes
  newedges?:
  / i: & intersectleftindexes[newedges[;0]; nodes] / only keep edges
	/ that match nodes
  i: & (newedges[;0] _in\: nodes) & (newedges[;1] _in\: nodes)
  newedges@: i
  maxnode: |/ nodes,edges[;0], edges[;1]
  mat: ((1+maxnode);(1+maxnode)) # 0
  / establish matrix
  i: 0
  while[i < #newedges
	mat[newedges[i;0]; newedges[i;1]]: 1
	mat[newedges[i;1]; newedges[i;0]]: 1
	i+: 1
  ]
  / compute transitive closure
  change: 1
  while[change
	oldmat: mat
	mat|: mat logicdot/:\: +mat
	change: ~ oldmat ~ mat
  ]
  :mat
}

/ spit out a graph such that 0 means no edge and 1 means an edge
spitadj:{[graph]
  convertline:{[line]
	i: & line < 0
	line[i]: 0
	i: & line > 0
	line[i]: 1
	x: -1 _ ,/($line) ,\: (" ")
	:x
  }
  y: convertline'graph
  z:(,$#graph), y
  :z
}


/ SECTION: INPUT DATA FROM FILE



/ Take input from an input file 
/ in the format:
/ #labels
/ name
/ #types
/ single line of types
/ #edges
/ line after line of pairs
/ This does not tolerate errors.
inputfromfile:{[filename]
  a: 0: filename
  currentlabel:: ()
  currenttype:: ()
  currentedge:: ()
  currentnodecount:: 0
  labelflag:: 0
  typeflag:: 0
  edgeflag:: 0
  labelsall:: ()
  adjall:: ()
  typesall:: ()
  linenum:: 0
  nonemptylinenum:: 0
  origedgeall::()
  justempty:: 1 / as if we've just seen an empty line. should process table
  a,: ,,""
  processline'a
  origedgeall,: ,reverseandnodup[currentedge] / last one
  countedges:{[edgelist] :[0 = #edgelist; 0; #?edgelist[;0]] }
  counts: countedges'origedgeall
  i: & (counts > #:'typesall) | (0 = counts) / get rid of edgeless graphs
  origedgeall:: origedgeall _di i
  typesall:: typesall _di i
  if[0 < #labelsall
    rejectlabels:: labelsall[i]
    labelsall:: labelsall _di i
  ]
  realoriglabel,: labelsall
  realorigedgeall,: origedgeall
  origedgeall:: setreduce'origedgeall
}

/ take edges each of which has a set of distances and reduce that
/ set to the cardinality of that set
setreduce:{[edges]
  setreplace:{[edge]
        if[3 < #edge
                multiedgeflag:: 1
                x :(edge[0], edge[1], 0.0 $ ($((#edge) - 2)), ("."),($(+/(2 _ edge))))
		:x
                / :(edge[0], edge[1], ,(2 _ edge)) / keep the whole graph
        ]
        :edge
  }
  :setreplace'edges
}


/ make sure that the edges are symmetric and no duplicates
/ add in a count of 1.
reverseandnodup:{[edges]
  if[0 = #edges; :()]
  i: & 2 = #:'edges
  edges[i],: 1
  newedges: edges
  newedges[;0]: edges[;1]
  newedges[;1]: edges[;0]
  :?edges,newedges
}


/ Handles one line of input at a time according to table schema above.
/ A little more space efficient than the previous version.
/ Note that one can put the weight as the third element of edges.
/ The default is 1.
processline:{[line]
 linenum+: 1
 emptyorblank:{[line] (0 = #delendblanks[line])}
 emptyflag: emptyorblank[line]
 if[~emptyflag
  nonemptylinenum+: 1
  justempty:: 0
  if[line[0] = "#"
    if[(("la") ~ line[1 2]) | (("LA") ~ line[1 2]) | (("La") ~ line[1 2])
	typeflag:: 0
	edgeflag:: 0
	labelflag:: 1
	if[(nonemptylinenum > 1)
		origedgeall,: ,reverseandnodup[currentedge]
		/ adjall,: ,edgetomatrix[nopath; currentnodecount; currentedge]
		currentedge::()
		currentnodecount:: 0
	]
    ]
    if[(("ty") ~ line[1 2]) | (("TY") ~ line[1 2]) | (("Ty") ~ line[1 2])
	typeflag:: 1
	edgeflag:: 0
	labelflag:: 0
	if[0 < #currentedge / need this if no labels
		/ but then need to handle case of no labels and
		/ some empty edgesets ???
		origedgeall,: ,reverseandnodup[currentedge]
		/ adjall,: ,edgetomatrix[nopath; currentnodecount; currentedge]
		currentedge::()
		currentnodecount:: 0
	]
	if[0 < #currentlabel 
		labelsall,: ` $ currentlabel
		currentlabel:: ()
	]
    ]
    if[(("ed") ~ line[1 2]) | (("ED") ~ line[1 2]) | (("Ed") ~ line[1 2])
	typeflag:: 0
	edgeflag:: 1
	labelflag:: 0
	if[0 = #currenttype; !-3] / should have types
	if[0 < #currenttype
		typesall,: , currenttype
		currentnodecount:: #currenttype
		currenttype:: ()
	]
    ]
  ]
  if[~ line[0] = "#"
	if[typeflag = 1
		currenttype,: . line
	]
	if[edgeflag = 1
		currentedge,: , . line
	]
	if[labelflag = 1
		currentlabel,: delendblanks[line]
	]
  ]
 ] / emptyflag
}

/ SECTION: DATA
nopath: -1 / distance meaning no link
if[manygraphs > 0 / only in this case do we need data
seed: 20
xx: seed _draw 100 / just to use up seed numbers. Change seed and
	/ you change the data output.
if[edgecount < 0; edgecount: 2 * nodecount]
/ edgecount: 4 * nodecount
adj1: (0 1 -1 -1
1 0 1 1
-1 1 0 -1
-1 1 -1 0)

adj2: (0 1 -1 -1 1
1 0 1 1 -1
-1 1 0 -1 -1
-1 1 -1 0 -1
1 -1 -1 -1 0)

adj2: (0 1 -1 -1 -1
1 0 1 1 -1
-1 1 0 -1 -1
-1 1 -1 0 -1
-1 -1 -1 -1 0)

adj2: (0 1 -1 -1 -1
1 0 1 -1 -1
-1 1 0 1 -1
-1 -1 1 0 -1
-1 -1 -1 -1 0)


label1: `lab1
adj1: randgen[nopath; nodecount; edgecount]
types1: randtypes[nodecount; numtypes] / types of the nodes
e: findneighbors[nopath; !#adj1 ; adj1]
w: findweights[nopath; !#adj1; adj1]
e2: permute[e] / has side effect to realalign, thus gives labels to other nodes
label2: `lab2
types2: types1
types2[realalign[;1]]: types1[realalign[;0]]
/ adj2: edgetomatrix[nopath; nodecount; e2]
if[0 < extranodes
 types2,: randtypes[extranodes; numtypes]
]
adj2: edgetomatrixextra[nopath; nodecount+extranodes; e2; extraedges; w]
outadj: ,spitadj[adj1]
outadj,: ,spitadj[adj2]
] / no longer generate data


/ SECTION: EXECUTION
/ TIME TESTING HARNESS (REMOVE / if you want this)
/ .time.set`.k
/ start: _t
/ END OF TIME TESTING HARNESS (REMOVE / if you want this)
out2: ()
start: _t


lower: 1 / reference pair must be at least lower
/ if[2 < #_i; lower: 0 $ _i[2]]
upper: 1 / reference pair must not be greater than upper
/ if[3 < #_i; upper: 0 $ _i[3]]
ratio: upper % lower
/ lower|: edgeweight / otherwise can't get a pair
	/ e.g. if specify distance of 1 and nothing is of that distance
upper|: _ ratio * lower 

minsim: 0.2 / minimum similarity measure required to be of interest
minsim: 0
 / this is for diameter searching
if[manygraphs > 0
 adjothers: ()
 typesothers: ()
 labelsothers: ` $ $ 1+!manygraphs
 do[manygraphs
   adjothers,: ,randgen[nopath; nodecount; edgecount]
   typesothers,: ,randtypes[nodecount; numtypes]
 ]
 labelsothers,: label2
 adjothers,: ,adj2
 typesothers,: ,types2
 origedge1: findneighbors[nopath; !#types1; adj1]
 origedgeothers: findneighbors[nopath]'[!:'#:'typesothers ; adjothers]
	/ ignoring weights
 vals1: computevalences[#types1; origedge1]
 valsothers: computevalences'[#:'typesothers; origedgeothers]
 / out2:findclosest[minsim; origedge1; types1; `;vals1; origedgeothers; typesothers; (); valsothers] / format of agency matrix is unpleasant
   out2,: findclosest[minsim; origedge1; types1; label1; vals1; origedgeothers; typesothers; labelsothers; valsothers]
]

if[manygraphs = -1
   inputfromfile[filenameforalltoall]
   label1: `
   labelsothers: ()
   if[0 < #labelsall
	label1: labelsall[0]
	labelsothers: 1 _ labelsall
   ]
   types1: typesall[0]
   origedge1: origedgeall[0]
   typesothers: 1 _ typesall
   origedgeothers: 1 _ origedgeall
   vals1: computevalences[#types1; origedge1]
   valsothers: computevalences'[#:'typesothers; origedgeothers]
   out2: findclosest[minsim; origedge1; types1; label1; vals1; origedgeothers; typesothers; labelsothers ; valsothers]
   / lastalign has the last alignment calculated by exec
   / For heuristic 4 it will have 2 depending on whether strength
   / is counted or not.
]
 
/ all against all
if[manygraphs = -2
   inputfromfile[filenameforalltoall]
  allagainstall:{[]
   label1: `
   labelsothers: ()
   valsall: computevalences'[#:'typesall; origedgeall]
   numgraphs: #origedgeall
   i: 0
   out2: ()
   while[i < numgraphs
      types1: typesall[i]
      origedge1: origedgeall[i]
      vals1: valsall[i]
      typesothers: typesall _di i
      origedgeothers: origedgeall _di i
      valsothers: valsall _di i
      if[0 < #labelsall
	label1: labelsall[i]
	labelsothers: labelsall _di i
      ]
      out2,: findclosest[minsim; origedge1; types1; label1; vals1; origedgeothers; typesothers; labelsothers; valsothers]
      i+: 1
  ]
  :out2
 }
 out2: allagainstall[]
  / if kmeans > 0, we try to find the kmeans centroids that induce 
  / kmeans partitions such that the minimum similarity radius in every
  / parition is maximized.
  if[kmeans > 0
     comps: ? ,/ clusterexpand'out2 / create (source,dest,dist,align)
     pair: findbestclusters[kmeans; comps[;!3]] / no align for clustering
     clusters: pair[0]
     / when reducedcentroids is 1, we replace centroids by subgraphs
     / that may give a better match to other nodes
     / then we rerun all against all to see how and 
     / whether clusters have improved
     if[reducecentroids
       / here we find the best subgraphs of each centroid to give
       / a better matching.
       clustersaslists: +pair[1]
       bestcentroids: findbesthead[comps]'clustersaslists
       jj: labelsall ?/: bestcentroids[;0] / where old labels were
       jj1: realoriglabel ?/: bestcentroids[;0]
       newlabels: ` $  ($bestcentroids[;0]) ,\: ("_reduced")
       bestcentroids[;0]: newlabels
       labelsall[jj]: newlabels
       realoriglabel[jj1]:: newlabels
       typesall[jj]: bestcentroids[;2]
       xx: reducenodenum'bestcentroids[;3]
       origedgeall[jj]: xx
	/ now modify originals
       realorigedgeall[jj1]:: xx 
		/ take edges and make node numbers as low as possible
		/ preserving order and therefore types.
       out2: allagainstall[]
       comps: ? ,/ clusterexpand'out2 / create (source,dest,dist,align)
       pair: findbestclusters[kmeans; comps[;!3]] / no align for clustering
       clusters2: pair[0]
     ]
  ]
]
 
/ manygraphs = -3 means that there are two files: "tempindb" (dbfile)
/ and "tempinprobes" (probefile)
/ We execute one probe at a time.
if[manygraphs = -3
   inputfromfile[dbfile]
   dbtypesall: typesall
   dblabelsall: labelsall
   dborigedgeall: origedgeall 
   dbvalsall: computevalences'[#:'dbtypesall; dborigedgeall]
   inputfromfile[probefile]
   numgraphs: #origedgeall
   i: 0
   out2: ()
   while[i < numgraphs
      types1: typesall[i]
      origedge1: origedgeall[i]
      vals1: computevalences[#types1; origedge1]
      if[0 < #labelsall
	label1: labelsall[i]
      ]
      out2,: findclosest[minsim; origedge1; types1; label1; vals1; dborigedgeall; dbtypesall; dblabelsall; dbvalsall]
      i+: 1
  ]
]
 
/ manygraphs = -3 means that there are two files: "tempindb" (dbfile)
/ and "tempinprobes" (probefile)
/ We execute one probe at a time.
  
  
/ inputs ?? incomplete
if[manygraphs = -4
   inputfromfile[filenameforalltoall]
   "temptypesall" 1: typesall
   "temporigedgeall" 1: origedgeall
      if[0 < #labelsall
	"templabelsall" 1: labelsall
      ]
]

/ take an input and use some to probe the others
if[manygraphs = -5
   inputfromfile[filenameforalltoall]
   label1: `
   labelsothers: ()
   numgraphs: #origedgeall
   valsall: computevalences'[#:'typesall; origedgeall]
   out2: ()
   somenumber: 100 & _ numgraphs % 2
   / somenumber: numgraphs / for testing: do all pairs
   chosen:  somenumber _draw -numgraphs
   j: 0
   while[j < #chosen
      i: chosen[j]
      types1: typesall[i]
      origedge1: origedgeall[i]
      vals1: valsall[i]
      typesothers: typesall _di i
      origedgeothers: origedgeall _di i
      valsothers: valsall _di i
      if[0 < #labelsall
	label1: labelsall[i]
	labelsothers: labelsall _di i
      ]
      out2,: findclosest[minsim; origedge1; types1; label1; vals1; origedgeothers; typesothers; labelsothers; valsothers]
      j+: 1
  ]
]
  
  

if[manygraphs = 0 / testing one against one
  zz: exec[adj1; adj2; types1; types2; label1; labelsothers[0]]


  / nodecount | edgecount | lower | upper | extraedges 
  / | extranodes | heuristic | numtypes 
  / | resultalign |resultglobal
  / || total time | time to build | time to compare | time to map and check

  out2,: ($nodecount),(" | ")
  out2,: ($edgecount),(" | ")
  out2,: ($lower),(" | ")
  out2,: ($upper),(" | ")
  out2,: ($extraedges),(" | ")
  out2,: ($extranodes),(" | ")
  out2,: ($heuristic),(" | ")
  out2,: ($numtypes),(" | ")
  out2,: ($zz[0]),(" | ")
  out2,: ($zz[1]),(" || ")
  out2,: ($t5 - start),(" | ")
  out2,: ($t1 - start),(" | ")
  out2,: ($t2 - t1),(" | ")
  out2,: ($t5 - t4),(" | ")
] / if manygraphs = 0

flattencomma:{[list]
  x:(-1) _ ,/($list) ,\: (",")
  :x
}

flattenspace: {[list]
  :("("), ($list[0]), (","), ($list[1]), (") ")
}

flatten:{[list]
  if[0 = #list;  :"No good matches"]
  if[0 = #list[0];  :"No good matches"]
  z: ()
  if[4 < #list
	z: flattenspace'list[4]
  ]
  nummatches: #list[3]
  firstmatch: * list[3]
  / list[3]: flattencomma[list[3]] / don't need because always singleton
  list[3]: *list[3]
  list[0 2]:list[2 0]
  x:(-1) _ ,/($list[0 1 2 3]) ,\: ("|")
  if[(0 < #z) & (1 = nummatches)
	x,: ("| Alignment: "), ,/z
  ]
  if[(0 < #z) & (1 < nummatches)
	x,: ("| Alignment to "), ($firstmatch) , (" is: "), ,/z
  ]
  :x
}

jj: & 1 < #:' out2
out2@: jj
y: flatten'out2
if[0 < #rejectlabels
  y,: ("Graphs in incorrect format: ")
  y,: flattencomma[rejectlabels]
]
y
z: ""
if[(manygraphs = -2) & (0 = #clusters) 
 z: ("A kmeans value of "), ($kmeans) , (" is too low to get ")
 z,: ("meaninful clusters.")
]
if[(manygraphs = -2) & (0 < #clusters) 
 z: ,("Best clusters are: ")
 z,: ,$clusters
 if[reducecentroids
   z,: ,("Best clusters with reduced centroids are: ")
   z,: ,$clusters2
   z,: ,("Essential nodes and edges of reduced centroids are: ")
   z,: bestcentroids
 ]
]
z
/ outadj
if[manygraphs > 0
 i: 0
 while[i < #outadj
   fileout: ("tmpgraph_"),($i)
   fileout 0: outadj[i]
   i+: 1
 ]
]
/ TIME TESTING HARNESS (REMOVE / if you want this)
/ .time.sum[]
/ xf: .TIME.f
/ xt: .TIME.t
/ xx: xf,'xt
/ zz: xx @ <xx[;1]
/ `show `.TIME
/ TIME TESTING HARNESS (ADD / if you want this)
\\
