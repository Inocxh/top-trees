all: top_trees.pdf

# LaTeX must be run multiple times to get references right
%.pdf: %.tex $(wildcard *.tex) bibliography.bib
	pdflatex $<
	bibtex $*
	while grep 'Rerun to get ' $*.log ; do pdflatex $< ; done

top_trees.pdf: top_trees.acr

%.acr: %.acn
	diff $*.acn $*-copy.acn || makeglossaries $*;
	cp $*.acn $*-copy.acn; done

%.acn: %.tex $(wildcard *.tex)
	pdflatex $<

clean:
	rm -f *.log *.dvi *.aux *.toc *.lof *.lot *.out *.bbl *.blg
	rm -f *.acn *.acr *.alg *.glsdef
	rm -f top_trees.pdf

.SECONDARY:
