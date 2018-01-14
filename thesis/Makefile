name=top_trees
all: $(name).pdf

.PHONY: FORCE

%.pdf: %.tex FORCE
	latexmk -pdf $<

clean:
	latexmk -C
	rm -f *.acn *.bbl *.ist *.lod
	rm -f $(name)-*.pdf $(name)-*.asy
