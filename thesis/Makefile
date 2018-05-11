name=top_trees
all: $(name).pdf abstract_czech.pdf abstract_english.pdf

.PHONY: FORCE

%.pdf: %.tex FORCE
	latexmk -pdf $<

clean:
	latexmk -C
	rm -f *.acn *.bbl *.ist *.lod
	rm -f $(name)-*.pdf $(name)-*.asy
	rm -f *.xmpdata $(name).pre *.xmpi
