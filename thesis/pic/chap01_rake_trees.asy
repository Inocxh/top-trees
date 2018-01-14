include "./pic/primitives.asy";

pair a,b,c,d;
a=(0,0); b=(1.5,0.2); c=(3,0); d=(4.5,0.2);

// Vertices and edges
vertex(a, "$a$", NW);
vertex(b, "$b$", NW);
vertex(c, "$c$", NW);
vertex(d, "$d$", NW);
edge(a,b);
edge(b,c);
edge(c,d);

subtree(b, "$B_L$", -20);
subtree(b, "$B_R$", 160);
subtree(c, "$C_L$", -20);
subtree(c, "$C_R$", 160);
