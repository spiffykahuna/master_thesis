 #!/bin/sh
echo 'We are in:'
pwd

rm *.bbl
rm $1.pdf
rm *.aux
rm *.glo
rm *.glg
rm *.ist
rm *.blg
rm *.gls
rm *.toc
rm *.log
#rm wordcount.tex
#./texcount_2_1.pl -1 Introduction.tex Aim.tex Contribution.tex BodyOfKnowledge.tex RESTful_POG.tex > wordcount.tex
#cat wordcount.tex |sed 's/\([0-9]*\)+.*/\1/' > wordcount2.tex
cp ../tex/*.tex ./
pdflatex -interaction=scrollmode --src-specials $1.tex
bibtex $1
pdflatex -interaction=scrollmode --src-specials $1.tex
#makeindex -s $1.ist -t $1.glg -o $1.gls $1.glo
makeglossaries $1
pdflatex -interaction=scrollmode --src-specials $1.tex

cp $1.pdf ../out/

