unitsize(1.2cm);
settings.outformat = "pdf";
defaultpen(fontsize(11pt));

real r = 0.06;
real noder = 0.15;

path vcut(path p) {
	pair begin = point(p, 0);
	pair end = point(p, length(p));
	path pp = firstcut(p, circle(begin, r)).after;
	return lastcut(pp, circle(end, r)).before;
}

// Draw primitives:

void vertex(pair pos, Label L="", align align=NoAlign) {
	fill(circle(pos, r));
	label(L, pos, align);
}

void node(pair pos, Label L="", align align=NoAlign) {
	fill(circle(pos, noder), gray(1));
	draw(circle(pos, noder));
	label(L, pos, align);
}

void edge(pair a, pair b, pen p=currentpen, arrowbar arrow=None, Label L="", align align=NoAlign) {
	draw(vcut(a--b), arrow=arrow, p);
	label(L, midpoint(a--b), align);
}

void subtree(pair pos, Label L="", align align=NoAlign, real angle=0) {
	pair a,b,c;
	a=pos+rotate(angle)*(-0.5,-1);
	b=pos+rotate(angle)*(0.5,-1);
	c=pos+rotate(angle)*(0,-0.6);
	draw(pos--a--b--pos);
	label(L, c);
}
