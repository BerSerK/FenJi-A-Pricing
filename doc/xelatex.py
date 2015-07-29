makefile=open("_build/latex/Makefile","r")
makefiletmp=open("_build/latex/Makefile~","w")

xeCJK="usepackage{xeCJK}\n \\setCJKmainfont[BoldFont=Heiti SC, ItalicFont=Heiti SC]{Heiti SC}\n\\setCJKmonofont[Scale=0.9]{Heiti SC}\n\\setCJKfamilyfont{song}[BoldFont=Heiti SC]{Heiti SC}\n\\setCJKfamilyfont{sf}[BoldFont=Heiti SC]{Heiti SC}\n\\begin{document}"
texfile=open("_build/latex/MonteCarloC.tex","r")
texfiletmp=open("_build/latex/MonteCarloC.tex~","w")
for l in makefile.readlines():
    l=l.replace('pdflatex','xelatex')
    makefiletmp.write(l)

for l in texfile.readlines():
    l=l.replace("begin{document}",xeCJK)
    l=l.replace("\\DeclareUnicodeCharacter{00A0}{\\nobreakspace}","");
    texfiletmp.write(l)
    
import os
os.system("mv _build/latex/Makefile~ _build/latex/Makefile")
os.system("mv _build/latex/MonteCarloC.tex~ _build/latex/MonteCarloC.tex")
