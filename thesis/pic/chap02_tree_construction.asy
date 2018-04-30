include "./pic/primitives.asy";

void connectEllipse(path f, pair a, pair b) {
	draw(firstcut(f..midpoint(a--b), getEllipse(a,b)).before, dotted, Arrow);
	drawEllipse(a, b);
}
void connectCircle(path f, pair a) {
	draw(firstcut(f..a, getCircle(a)).before, dotted, Arrow);
	drawCircle(a);
}

////////////////////////////////////////////////////////////////////////////////

pair shift = (0,2);

// 0. Level
pair a,b,c,d,e1,e2,f,g,h;
a=(0,1); b=(0.75,0.5); c=(0,0); d=(2.5,1);
e1=(1.75,0.5); e2=(2.5,0); f=(3.5,-0.5); g=(3.5,0.5); h=(4.5,0.5);

// 1. Level
pair cc, ab, e1d, e2f, gh;
cc = c+shift; ab = b+shift; e1d = e1+shift; e2f = e2+shift; gh = g+shift;

shift = (0,1.5);
// 2. Level
pair cb, e1e2, ghgh;
cb = ab+shift; e1e2 = e1d + shift; ghgh = gh + shift - (0.5,0);

// 3. Level
pair ce1, ghghgh;
ce1 = e1e2 + shift;
ghghgh = ghgh + shift - (0.5,0);

// 4. Level
pair ch = midpoint(ce1--ghghgh) + shift;

////////////////////////////////////////////////////////////////////////////////

// Draw from top

// 4. Level
vertex(ch, "$c$-$h$", N);

connectEllipse(ch, ce1, ghghgh);

// 3. Level
vertex(ce1, "$c$-$e_1$", NW);
vertex(ghghgh, "$g$-$h$", NE);
edge(ce1, ghghgh);

connectEllipse(ce1..ce1-(.1,.2), cb, e1e2);
connectCircle(ghghgh..ghghgh+(.1,-.2), ghgh);

// 2. Level
vertex(cb, "$c$-$b$", NW);
vertex(e1e2, "$e_1$-$e_2$", NE);
vertex(ghgh, "$g$-$h$", NE);
edge(cb, e1e2);
edge(e1e2, ghgh);

connectEllipse(cb..cb-(.1,.2), cc, ab);
connectEllipse(e1e2..e1e2+(.1,-.2), e1d, e2f);
connectCircle(ghgh..ghgh+(.1,-.2), gh);

// 1. Level
vertex(cc, "$c$", NW);
vertex(ab, "$a$-$b$", N);
vertex(e1d, "$e_1$-$d$", N);
vertex(e2f, "$e_2$-$f$", S);
vertex(gh, "$g$-$h$", NE);
edge(cc, ab);
edge(ab, e1d);
edge(e1d, e2f, dotted);
edge(e2f, gh);

connectCircle(cc..(cc-(.5,.5)), c);
connectEllipse(ab..ab+(.1,-.2), a, b);
connectEllipse(e1d..e1d-(.1,.2), e1, d);
connectEllipse(e2f..e2f+(.1,-.2), e2, f);
connectEllipse(gh..gh+(.1,-.2), g, h);


// 0. Level
vertex(a, "$a$", N);
vertex(b, "$b$", NE);
vertex(c, "$c$", S);
vertex(d, "$d$", N);
vertex(e1, "$e_1$", NW);
vertex(e2, "$e_2$", S);
vertex(f, "$f$", S);
vertex(g, "$g$", S);
vertex(h, "$h$", S);
edge(a,b);
edge(b,c);
edge(b,e1);
edge(e1,d);
edge(e1,e2, dotted);
edge(e2,f);
edge(e2,g);
edge(g,h);
