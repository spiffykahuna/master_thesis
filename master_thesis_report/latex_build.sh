#!/bin/sh
echo 'We are in:'
pwd

rm *.bbl
rm "$1".pdf
rm *.aux
rm *.glo
rm *.glg
rm *.ist
rm *.blg
rm *.gls
rm *.toc
rm *.log
rm *.sty

#rm wordcount.tex

cp ../tex/*.tex ./
cp ../resource/*.sty ./
pdflatex -interaction=scrollmode --src-specials -shell-escape "$1".tex
bibtex "$1"
pdflatex -interaction=scrollmode --src-specials -shell-escape "$1".tex
#makeindex -s $1.ist -t $1.glg -o $1.gls $1.glo
makeglossaries "$1"
pdflatex -interaction=scrollmode --src-specials -shell-escape "$1".tex
cp "$1".pdf ../out/

echo 'OUTPUT_FOLDER:'
ls -lash ../out/