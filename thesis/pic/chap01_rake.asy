include "./pic/primitives.asy";

void rake_draw(pair v, pair u, pair w, bool right=false) {
	// Outgoing edges
	draw(v--(v+(.4,.1)));
	draw(v--(v+(-.4,.1)));

	pen dotpen = linewidth(3);;

	pair R = (right ? u : w);
	draw(R--(R+(.4,-.1)));
	draw(R--(R+(-.4,-.1)));

	// Vertices and edges
	vertex(v, "$v$", N);
	vertex(u, "$u$", NW);
	vertex(w, "$w$", NE);
	edge(v,u, "$C_1$", NW);
	edge(v,w, "$C_2$", NE);

	// Rake arrow
	if (right) draw(midpoint(v--w){SW}..midpoint(v--u), dashed, Arrow);
	else draw(midpoint(v--u){SE}..midpoint(v--w), dashed, Arrow);

	// After rake
	pair shift= (right ? (6,0) : (5,0));
	pair vv=v+shift;
	pair uu=u+shift;
	pair ww=w+shift;

	// Outgoing edges
	draw(vv--(vv+(.4,.1)));
	draw(vv--(vv+(-.4,.1)));
	pair RR = (right ? uu : ww);
	draw(RR--(RR+(.4,-.1)));
	draw(RR--(RR+(-.4,-.1)));

	vertex(vv, "$v$", N);
	if (right) {
		vertex(uu, "$u$", NW);
		edge(vv,uu, "$C$", NW);
	} else {
		vertex(ww, "$w$", NE);
		edge(vv,ww, "$C$", NE);
	}
}

// Left rake

pair v,u,w;
v=(0,1); u=(-1,0); w=(1,0);
rake_draw(v,u,w);

// Label arrow
draw((2,.7)--(3.5,.7), Arrow(HookHead), L=Label("\it Left rake", MidPoint));

// Right rake

pair shift=(0,-1.7);
pair v=v+shift;
pair u=u+shift;
pair w=w+shift;

rake_draw(v,u,w, true);

// Label arrow
draw((2,.7)+shift--(3.5,.7)+shift, Arrow(HookHead), L=Label("\it Right rake", MidPoint));
