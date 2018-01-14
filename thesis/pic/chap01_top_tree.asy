include "./pic/clusters.asy";

pair a,b,c,d,e,f,g;
a=(0,1); b=(0.75,0.5); c=(0,0);
d=(2.5,1); e=(1.75,0.5); f=(2.5,0); g=(2.5,0.5);

// Vertices and edges
vertex(a, "$a$", N);
vertex(b, "$b$", NE);
vertex(c, "$c$", S);
vertex(d, "$d$", N);
vertex(e, "$e$", NW);
vertex(f, "$f$", S);
vertex(g, "$g$", E);
edge(a,b);
edge(b,c);
edge(b,e);
edge(e,d);
edge(e,f);
edge(e,g);

// Base clusters
Cluster ab = makeBase("$a,b$");
Cluster bc = makeBase("$b,c$");
Cluster be = makeBase("$b,e$");
Cluster ed = makeBase("$e,d$");
Cluster ef = makeBase("$e,f$");
Cluster eg = makeBase("$e,g$");

// b-d path
Cluster r = makeRake("$e,g$", ef, eg);
Cluster rr = makeRake("$e,d$", r, ed);
Cluster bd = makeCompress("$b[e]d$", be, rr);

// a-c path
Cluster rrr = makeRake("$a,b$", bd, ab);
Cluster root = makeCompress("$a[b]c$", rrr, bc);

draw(root, (7,2.5));
