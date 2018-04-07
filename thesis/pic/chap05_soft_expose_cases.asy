include "./pic/clusters.asy";

// First case
currentpen=dotted;
Cluster left = makeCompress("$u,x$");
Cluster right = makeCompress("$x,v$");
currentpen=linewidth(1.5);
Cluster root = makeCompress("$u[x]v$", left, right);

currentpen=dotted;
draw(root, (0,0));
label("\it Case 1", (0,.5), N);

// Second case
left = makeCompress("$a,u$");
currentpen=linewidth(1.5);
right = makeCompress("$u,v$");
currentpen=defaultpen;
root = makeCompress("$a[u]v$", left, right);

currentpen=dotted;
draw(root, (4,0));
label("\it Case 2", (4,.5), N);

// Third case
currentpen=dotted;
left = makeCompress("$a,u$");
currentpen=linewidth(1.5);
Cluster rightright = makeCompress("$u,v$");
currentpen=dotted;
Cluster rightleft = makeCompress("$v,b$");
currentpen=defaultpen;
right = makeCompress("$u[v]b$", rightleft, rightright);
root = makeCompress("$a[u]b$", left, right);

currentpen=dotted;
draw(root, (8,0));
label("\it Case 3", (8,.5), N);
