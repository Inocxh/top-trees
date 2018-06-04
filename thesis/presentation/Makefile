name=top_trees_presentation
pics=$(addsuffix .pdf, chap01_compress chap01_rake chap01_top_tree chap05_splice2 chap06_tree_mapping)

all: $(name).pdf

$(name).pdf: $(pics)

.PHONY: all clean FORCE

%.pdf: pic/%.asy
	asy $<

%.pdf: %.tex FORCE
	latexmk -pdf $<

clean:
	latexmk -C
	rm -f *.pdf
	rm -f *.acn *.bbl *.ist *.lod *.nav *.snm *.vrb
	rm -f $(name)-*.pdf $(name)-*.asy
	rm -f *.xmpdata $(name).pre *.xmpi

