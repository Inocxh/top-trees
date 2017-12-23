all: top_trees.pdf

.PHONY: FORCE

%.pdf: %.tex FORCE
	latexmk -pdf $<

clean:
	latexmk -C
	rm -f *.acn *.bbl *.ist *.lod

