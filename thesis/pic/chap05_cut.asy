include "./pic/clusters.asy";

treeLevelStep = 1.3cm;

Cluster A = makeBase("$A$");
Cluster B = makeBase("$B$");
Cluster C = makeBase("$C$");
Cluster D = makeBase("$D$");

Cluster left = makeCompress("$a,u$");
currentpen=linewidth(1.5);
Cluster rightright = makeCompress("$u,v$");
currentpen=defaultpen;
Cluster rightleft = makeCompress("$v,b$");
Cluster right = makeCompress("$u[v]b$", rightleft, rightright, C, D);
Cluster root = makeCompress("$a[u]b$", left, right, A, B);

draw(root, (0,0));

////////////////////////////////////////////////////////////////////////////////

draw((2.5,-1)--(4,-1), Arrow(HookHead), L=Label("\it Cut$(u,v)$", MidPoint));

////////////////////////////////////////////////////////////////////////////////

A = makeBase("$A$");
B = makeBase("$B'$");
C = makeBase("$C$");
D = makeBase("$D'$");

Cluster firstLeft = makeCompress("$a,u$");
Cluster firstRight = makeCompress("$u,x$");
Cluster firstRoot = makeCompress("$a[u]x$", firstLeft, firstRight, A, B);

draw(firstRoot, (6,0.5));

Cluster secondLeft = makeCompress("$v,b$");
Cluster secondRight = makeCompress("$v,y$");
Cluster secondRoot = makeCompress("$b[v]y$", secondLeft, secondRight, C, D);

draw(secondRoot, (6,-1.5));
