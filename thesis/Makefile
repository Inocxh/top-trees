all: top_trees.pdf

# LaTeX must be run multiple times to get references right
%.pdf: %.tex $(wildcard *.tex) bibliography.bib
	pdflatex $<
	bibtex $*
	while grep 'Rerun to get ' $*.log ; do pdflatex $< ; done

clean:
	rm -f *.log *.dvi *.aux *.toc *.lof *.lot *.out *.bbl *.blg
	rm -f top_trees.pdf

.SECONDARY:
