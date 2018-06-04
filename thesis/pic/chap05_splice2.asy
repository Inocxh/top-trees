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

////////////////////////////////////////////////////////////////////////////////

draw((2.5,2)--(4,2), Arrow(HookHead), L=Label("\it Splice", MidPoint));

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
