include "./pic/primitives.asy";

real clusterMinWidth = 0cm;
real treeLevelStep   = 1cm;
real treeNodeStep    = 0.3cm;

struct Cluster {
	Cluster parent = null;
	Cluster left = null;
	Cluster right = null;
	Cluster foster_left = null;
	Cluster foster_right = null;

	frame content;

	pair pos;
	real adjust;
}

Cluster makeCluster(frame f) {
	Cluster node = new Cluster;
	node.content = f;
	return node;
}
Cluster makeCompress(Label L="", Cluster left=null, Cluster right=null, Cluster foster_left=null, Cluster foster_right=null) {
	frame f;
	box(f, L, xmargin=0.1cm);
	Cluster c = makeCluster(f);
	if (left != null) {
		c.left = left;
		left.parent = c;
	}
	if (right != null) {
		c.right = right;
		right.parent = c;
	}
	if (foster_left != null) {
		c.foster_left = foster_left;
		foster_left.parent = c;
	}
	if (foster_right != null) {
		c.foster_right = foster_right;
		foster_right.parent = c;
	}
	return c;
}
Cluster makeRake(Label L="", Cluster left, Cluster right) {
	frame f;
	box(f, L, xmargin=0.1cm, FillDraw(gray(0.8)));
	Cluster c = makeCluster(f);
	c.left = left;
	left.parent = c;
	c.right = right;
	right.parent = c;
	return c;
}
Cluster makeBase(Label L="") {
	frame f;
	ellipse(f, L);
	return makeCluster(f);
}


real layout(int level, Cluster node ) {
	if (node.left == null && node.right == null
	&& node.foster_left == null && node.foster_right == null) {
		return max( clusterMinWidth, (max(node.content)-min(node.content)).x );
	}

	real sumWidth = 0;

	if (node.foster_left != null) {
		real w = layout(level+1, node.foster_left);
		node.foster_left.pos = (sumWidth + w/2, -level*treeLevelStep);
		sumWidth += w + treeNodeStep;
	}
	if (node.left != null) {
		if (node.foster_left != null) sumWidth -= treeNodeStep;
		real w = layout(level+1, node.left);
		node.left.pos = (sumWidth + w/2, -level*treeLevelStep);
		sumWidth += w + treeNodeStep;
	}
	if (node.foster_right != null) {
		real w = layout(level+1, node.foster_right);
		node.foster_right.pos = (sumWidth + w/2, -level*treeLevelStep);
		sumWidth += w + treeNodeStep;
	}
	if (node.right != null) {
		if (node.foster_right != null) sumWidth -= treeNodeStep;
		real w = layout(level+1, node.right);
		node.right.pos = (sumWidth + w/2, -level*treeLevelStep);
		sumWidth += w + treeNodeStep;
	}

	real midPoint = (sumWidth - treeNodeStep) / 2;
	if (node.foster_left != null) node.foster_left.adjust = -midPoint;
	if (node.left != null) node.left.adjust = -midPoint;
	if (node.foster_right != null) node.foster_right.adjust = -midPoint;
	if (node.right != null) node.right.adjust = -midPoint;

	return max(
		(max(node.content)-min(node.content)).x,
		sumWidth - treeNodeStep
	);
}


void drawAll(Cluster node, frame f) {
	pair posAdjust;
	if(node.parent != null) posAdjust = (node.parent.pos.x + node.adjust, 0);
	else posAdjust = (node.adjust, 0);

	node.pos += posAdjust;

	node.content = shift(node.pos)*node.content;
	add(f, node.content);

	if( node.parent != null ) {
		path p = point(node.content, N)--point(node.parent.content,S);
		if (node == node.parent.foster_left || node == node.parent.foster_right)
			draw(f, p, dotted);
		else draw(f, p, currentpen);
	}

	if (node.foster_left != null) drawAll(node.foster_left, f);
	if (node.left != null) drawAll(node.left, f);
	if (node.foster_right != null) drawAll(node.foster_right, f);
	if (node.right != null) drawAll(node.right, f);
}

void draw(Cluster root, pair pos ) {
	frame f;

	root.pos = (0, 0);
	layout(1, root);

	drawAll(root, f);

	add(f, pos);
}
