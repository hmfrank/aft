.PHONY: all continuous clean

all:
	latexmk -xelatex -pdf thesis.tex

continuous:
	latexmk -xelatex -pvc -pdf thesis.tex

clean:
	rm -f *.aux *.bbl *.blg *.fdb_latexmk *.fls *.lof *.log *.lot *.out *.pdf *.toc chapter/*.aux
