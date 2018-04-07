include "./pic/clusters.asy";

clusterMinWidth = 0.5cm;
treeLevelStep = 1.3cm;

pair a = (-1.5, 3);
pair v = (0, 2.5);
pair b = (1.5, 3);
pair c = (0, 1);

currentpen=dotted;
subtree(v, "$A$", 180);
subtree(v, "$B$", 315);
subtree(v, "$C$", 45);
currentpen=defaultpen;

vertex(a, "$a$", N);
vertex(b, "$b$", N);
vertex(c, "$c$", E);
vertex(v, "$v$", NE);
edge(a, v);
edge(v, b);
edge(v, c, dotted);

// Base clusters
Cluster A = makeBase("$A$");
Cluster B = makeBase("$B$");
Cluster C = makeBase("$C$");
Cluster av = makeCompress("$a,v$");
Cluster vb = makeCompress("$v,b$");
Cluster vc = makeCompress("$v,c$");
Cluster rake1 = makeRake("\phantom{m}", vc, C);
Cluster rake2 = makeRake("\phantom{m}", B, rake1);
Cluster root = makeCompress("$a[v]b$", av, vb, A, rake2);

draw(root, (0,0));

////////////////////////////////////////////////////////////////////////////////

draw((2.5,.5)--(4,.5), Arrow(HookHead), L=Label("\it Splice", MidPoint));

////////////////////////////////////////////////////////////////////////////////

pair shift = (6.5,0);
a = a+shift;
b = b+shift;
c = c+shift;
v = v+shift;

currentpen=dotted;
subtree(v, "$A$", 180);
subtree(v, "$B$", 315);
subtree(v, "$C$", 45);
currentpen=defaultpen;

vertex(a, "$a$", N);
vertex(b, "$b$", N);
vertex(c, "$c$", E);
vertex(v, "$v$", NE);
edge(a, v);
edge(v, b, dotted);
edge(v, c);

A = makeBase("$A$");
B = makeBase("$B$");
C = makeBase("$C$");
av = makeCompress("$a,v$");
vb = makeCompress("$v,b$");
vc = makeCompress("$v,c$");
rake1 = makeRake("\phantom{m}", A, vb);
rake2 = makeRake("\phantom{m}", rake1, C);
root = makeCompress("$a[v]c$", av, vc, rake2, B);

draw(root, shift);
