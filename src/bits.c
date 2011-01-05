#include "bits.h"

void init437(void)
{
	const char *xt[128] = {"Ç", "ü", "é", "â", "ä", "à", "å", "ç", "ê", "ë", "è", "ï", "î", "ì", "Ä", "Å", "É", "æ", "Æ", "ô", "ö", "ò", "û", "ù", "ÿ", "Ö", "Ü", "¢", "£", "¥", "₧", "ƒ", "á", "í", "ó", "ú", "ñ", "Ñ", "ª", "º", "¿", "⌐", "¬", "½", "¼", "¡", "«", "»", "░", "▒", "▓", "│", "┤", "╡", "╢", "╖", "╕", "╣", "║", "╗", "╝", "╜", "╛", "┐", "└", "┴", "┬", "├", "─", "┼", "╞", "╟", "╚", "╔", "╩", "╦", "╠", "═", "╬", "╧", "╨", "╤", "╥", "╙", "╘", "╒", "╓", "╫", "╪", "┘", "┌", "█", "▄", "▌", "▐", "▀", "α", "ß", "Γ", "π", "Σ", "σ", "µ", "τ", "Φ", "Θ", "Ω", "δ", "∞", "φ", "ε", "∩", "≡", "±", "≥", "≤", "⌠", "⌡", "÷", "≈", "°", "∙", "·", "√", "ⁿ", "²", "■", " "};
	xatiles=xt;
}

char * getl(FILE *fp)
{
	char * lout = (char *)malloc(81);
	int i=0;
	signed int c;
	while(!feof(fp))
	{
		c=fgetc(fp);
		if((c==10)||(c==EOF))
			break;
		if(c!=0)
		{
			lout[i++]=c;
			if((i%80)==0)
			{
				if((lout=(char *)realloc(lout, i+81))==NULL)
				{
					printf("\nNot enough memory to store input!\n");
					free(lout);
					return(NULL);
				}
			}
		}
	}
	lout[i]=0;
	char *nlout=(char *)realloc(lout, i+1);
	if(nlout==NULL)
	{
		return(lout); // it doesn't really matter (assuming realloc is a decent implementation and hasn't nuked the original pointer), we'll just have to temporarily waste a bit of memory
	}
	return(nlout);
}
