include "./pic/primitives.asy";

pair a,b,c;

currentpen+=linewidth(0.8);
pen outerpen = Dotted+linewidth(0.8);

label("\it Cluster degree:", (-1,2));

// No neighbours
// 1
a=(0.5,1);
label("0", a+(0,1));
node(a);
// 2
b=(0,0); c=(1,0);
edge(b,c);
node(b);
node(c);

// One neighbour
// 1
pair shift=(2.5,0);
a+=shift;
label("1", a+(0,1));
edge(a,a+(0.35,0.35), outerpen);
node(a);
// 2
b+=shift; c+=shift;
edge(b,c);
edge(c,c+(0.35,0.35), outerpen);
node(b);
node(c);

// Two neighbours
// 1
a+=shift;
label("2", a+(0,1));
edge(a,a+(-0.35,0.35), outerpen);
edge(a,a+(0.35,0.35), outerpen);
node(a);
// 2a
b+=shift; c+=shift;
edge(b,c);
edge(b,b+(-0.35,0.35), outerpen);
edge(c,c+(0.35,0.35), outerpen);
node(b);
node(c);
// 2b
b-=(0,1); c-=(0,1);
edge(b,c);
edge(b,b+(-0.35,0.35), outerpen);
edge(b,b+(-0.35,-0.35), outerpen);
node(b);
node(c);

// Three neighbours
a+=shift;
label("3", a+(0,1));
edge(a,a+(-0.35,0.35), outerpen);
edge(a,a+(0.35,0.35), outerpen);
edge(a,a+(0,-0.45), outerpen);
node(a);
