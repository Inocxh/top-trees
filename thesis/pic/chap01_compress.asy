include "./pic/primitives.asy";

pair u,v,w;
u=(0,0); v=(1,0.3); w=(2,0);

// Outgoing edges
draw(u--(u+(-.3,.15)));
draw(u--(u+(-.3,-.15)));
draw(w--(w+(.3,.15)));
draw(w--(w+(.3,-.15)));

// Vertices and edges
vertex(u, "$u$", N);
vertex(v, "$v$", N);
vertex(w, "$w$", N);
edge(u,v, "$C_1$", S);
edge(v,w, "$C_1$", S);

// Label arrow
draw((3.2,.2)--(4.7,.2), Arrow(HookHead), L=Label("\it Compress", MidPoint));

// After compress
pair shift= (6,0);
u=(0,0)+shift;
w=(1,0)+shift;

// Outgoing edges
draw(u--(u+(-.3,.15)));
draw(u--(u+(-.3,-.15)));
draw(w--(w+(.3,.15)));
draw(w--(w+(.3,-.15)));

vertex(u, "$u$", N);
vertex(w, "$w$", N);
edge(u,w, "$C$", S);
