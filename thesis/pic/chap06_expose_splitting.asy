include "./pic/clusters.asy";

treeLevelStep   = 1.5cm;

currentpen=dotted;

// Base clusters
Cluster a = makeCompress("$a$");
Cluster b = makeCompress("$b$");
Cluster c = makeCompress("$c$");
Cluster d = makeCompress("$d$");
Cluster e2 = makeCompress("$e_1$");
Cluster e1 = makeCompress("$e_2$");
Cluster f = makeCompress("$f$");

Cluster g = makeCompress("$g$");
Cluster h = makeCompress("$h$");

// Second level
Cluster cc = makeCompress("$c$", c);
Cluster ab = makeCompress("$a$-$b$", a, b);
currentpen=linewidth(1.5);
Cluster e1d = makeCompress("$e_1$-$d$", e1, d);
Cluster e2f = makeCompress("$e_2$-$f$", e2, f);
currentpen=dotted;
Cluster gh = makeCompress("$g$-$h$", g, h);

// Third level
currentpen=linewidth(1.5);
Cluster cb = makeCompress("$c$-$b$", cc, ab);
currentpen=dotted;
Cluster e1e2 = makeCompress("$e_1$-$e_2$", e1d, e2f);
Cluster ghgh = makeCompress("$g$-$h$", gh);

// Fourth level
Cluster ce2 = makeCompress("$c$-$e_2$", cb, e1e2);
currentpen=linewidth(1.5);
Cluster ghghgh = makeCompress("$g$-$h$", ghgh);

// Fifth level
currentpen=dotted;
Cluster ch = makeCompress("$c$-$h$", ce2, ghghgh);

draw(ch, (0,0));
