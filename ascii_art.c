/*
* ASCII Art: Real-time ASCII Art Rendering Library.
* Copyright (C) PixLab. https://pixlab.io/art
* Version 1.2
* For information on licensing, redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES
* please contact:
*       support@pixlab.io
*       contact@pixlab.io
* or visit:
*     https://pixlab.io/art
*/
/*
* An Implementation based on the paper:
* > N. Markus, M. Fratarcangeli, I. S. Pandzic and J. Ahlberg, "Fast Rendering of Image Mosaics and ASCII Art", Computer Graphics Forum, 2015, http://dx.doi.org/10.1111/cgf.12597
*/
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ascii_art.h>
#include <stdlib.h>
/*
 * This is the output hex model generated during the training phase. 
 * It contains both the codebook and the decision tree that let you
 * render your images or video frames at Real-time.
 *
 * The model can be downloaded from: https://pixlab.io/art
 */
static const unsigned char zBin[] = {
#if __has_include("ascii_art.hex") 
#include "ascii_art.hex"
#define  ascii_include 1
#else
#define ascii_include 0
#endif
};
/*
 * Glyph table.
 */
 const unsigned char glyph_char_table[] =
{
	' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~'
};

int8_t *arr_to_ptr(const uint8_t pack[]) {
	int ii=0;
	for(;pack[ii]!=0;ii++) ;
	uint8_t *ptr = malloc(sizeof(uint8_t)*ii);
	for(int i=0;i<ii;i++) ptr[i] = pack[i];
	return ptr;
}

/*
* Portion based on the work of Nenad Markus n3ar.
*/
void parse_art_model_(uint8_t** ppixels, int* n, int* nrows, int* ncols, count_t *packi, int32_t** tree, const uint8_t *pack)
{
	int count = 0;
	for(;*(int*)(pack+count)!=0;count++);
	int i, k;
	long int nn  = *(int*)(pack + (0* sizeof(int)));
	*n = *(int*)&pack[0];
	*nrows = *(int*)(pack + (1* sizeof(int)));
	*ncols = *(int*)(pack + (2 * sizeof(int)));
	packi->count_i = count;
	k = 3 * sizeof(int);
#if(ascii_include)
	for (i = 0; i < *n; ++i) {
#else
	for (i = 0; i < count; ++i) {
#endif
		ppixels[i] = (uint8_t*)&pack[k];
		k = k + (*nrows / *ncols);
	}
	packi->count_k = k;
	*tree = (int32_t*)(pack+k);
	for(int ii=0;ii<k;ii++) {
		*((*tree)+ii) = *(int32_t*)(pack+k+1);
	}
}

void parse_tree(int32_t **tree) {
		
}

void parse_art_model(uint8_t** ppixels, int* n, int* nrows, int* ncols, int32_t** tree, const uint8_t pack[])
{
	int i, k;
	*n = *(int*)&pack[0 * sizeof(int)];
	*nrows = *(int*)&pack[1 * sizeof(int)];
	*ncols = *(int*)&pack[2 * sizeof(int)];
	k = 3 * sizeof(int);
	for (i = 0; i < *n; ++i) {
		ppixels[i] = (uint8_t*)&pack[k];
		k = k + *nrows**ncols;
	}
	*tree = (int32_t*)&pack[k];
}

#define BINTEST(r, c, t, pixels, ldim) ( (pixels)[((r)*(ldim))+(c)] > (t) )
/*
* Portion based on the work of Nenad Markus n3ar. 
*/
 int get_tree_output(int32_t* tree, uint8_t* pixels, int ldim)
{
	uint8_t* n = (uint8_t*)&tree[1];
	int nodeidx = 0;
	while (n[0] == 1) /* while we are at a nonterminal node */
	{

		if (0 == BINTEST(n[1], n[2], n[3], pixels, ldim))
			nodeidx = 2 * nodeidx + 1;
		else
			nodeidx = 2 * nodeidx + 2;
		n = (uint8_t*)&tree[1 + nodeidx];
	}
	return n[1]; 
}
/*
* Portion based on the work of Nenad Markus n3ar. 
*/
 void compute_index_matrix(ascii_render *pRender, uint8_t pixels[], int nrows, int ncols, int ldim)
{
	int i = 0;
	int r, c;
	for (r = 0; r < nrows; r += pRender->nRows) {
		for (c = 0; c < ncols; c += pRender->nCols) {
			if (pRender->pTree)
				pRender->zMatrix[i] = get_tree_output(pRender->pTree, &pixels[r*ldim + c], ldim);
			else
				pRender->zMatrix[i] = 0;
			++i;
		}
	}
}
/*
* Portion based on the work of Nenad Markus n3ar. 
*/
 void rc_clahem(uint8_t imap[], uint8_t img[], int i0, int j0, int i1, int j1, int ldim, uint8_t s)
{
#define NBINS 256
	double p[NBINS];
	double P[NBINS];
	int i, j, k;
	int nrows, ncols;

	nrows = i1 - i0 + 1;
	ncols = j1 - j0 + 1;

	for (i = 0; i<NBINS; ++i)
		p[i] = 0.0;

	/* compute histogram */
	for (i = i0; i <= i1; ++i)
		for (j = j0; j <= j1; ++j)
		{
			k = img[i*ldim + j];

			p[k] = p[k] + 1.0 / (nrows*ncols);
		}

	/* clip the histogram (ideally, we should do a few iterations of this) */
	for (k = 0; k<NBINS; ++k)
	{
		if (p[k] >= (double)s / NBINS)
		{
			double d;

			d = p[k] - (double)s / NBINS;

			p[k] = (double)s / NBINS;

			/* redistribute d */
			for (i = 0; i<NBINS; ++i)
				p[i] += d / NBINS;
		}
	}

	/* compute cumulative histogram */
	P[0] = p[0];
	for (i = 1; i<NBINS; ++i)
		P[i] = P[i - 1] + p[i];

	/* compute intensity map */
	for (k = 0; k<NBINS; ++k)
		imap[k] = (uint8_t)((NBINS - 1)*P[k]);
}

// section based from worthless443  (end)
int *check_for_same(int *tree, int arr[])  {
	if(tree[0]==0) return tree;
	arr[0] =  tree[0];
	arr[1] = tree[1];
	int *arr_r ;
	int size, match;
	//if(arr[1] > 1 || arr[0] > 1) arr_r = check_for_same(tree+3, arr);
	arr_r = check_for_same(tree+3, arr);
	for(size=0;*(int*)(arr_r+size);size++); //printf("%d ", arr_r[size]);
	printf("%d\n",arr_r[size-1]);
	//printf("%d\n", size);
	//arr[0] = (arr_r[size]==tree[1]);
	//arr[1] = (arr_r[size-1]==tree[0]);

	//match = arr[0]==1 && arr[1]==0;
	//printf("%d\n", arr_r[0]);
	return tree;
} 

int *arr_without_zeros(int *tree, int size) {
	int *out = malloc(8*size);
	for(int i=0,j=0;i<size;i++) {
			if(tree[i]!=51) {
			out[j] = tree[i];
			j++;
		}
	}
	return out;
}

int *check_for_same_(int *tree, int **arr, int **out, int count)  {
	if(tree[0]==0) {
		return tree;
	}
	for(int i=1;*(tree+i);i++) { 
			if(tree[0] == tree[i])  tree[i] = 51;
	}
	arr[count] = tree;
	*out = arr[0]; // idk why save it as it is always at idx 0
	check_for_same_(tree+1, arr, out, ++count);
      	return tree;
}
int get_size(int *ptr) {
	int i=1;
	for(;*(ptr+i);i++);
	return i;
}

void print_tree(int *tree,int size) {
    	for(int k=0;k<size;k++) printf("%d ", tree[k]);
	printf("\n");
}

int check_same(int *tree) {
	int i;
	int *t;
	int size = get_size(tree);
	int *arr[size], *out;
	t = check_for_same_(tree,arr, &out,0);
	out = arr_without_zeros(out,size);
	
	//for(int i=0;i<get_size(tree);i++) if(out[i]==0) return 1;
	printf("%d : %d",get_size(out), get_size(tree));
	return (get_size(out)<get_size(t)) ? 1 : 0;
}

#define getSize(a) sizeof(a)/sizeof(int)
int check_zeros(int *arr) {
	int i=0,j=0;
	while(arr[i]!=-1) {
		if(arr[i]==0) {
			arr = arr + i;
			j++;
			i=0;
		}
		i++;
	}
	return j;
}
// section based from worthless443 (end)
//

/*
* Portion based on the work of Nenad Markus n3ar. 
*/
 void clahe_preprocess(uint8_t out[], uint8_t in[], int nrows, int ncols, int di, int dj, uint8_t s)
{
	int ldim = ncols;
#define MAXDIVS 16
	uint8_t imaps[MAXDIVS][MAXDIVS][NBINS];

	int ics[MAXDIVS], jcs[MAXDIVS];

	int i, j, k, l, i0, j0, i1, j1, I, J;

	uint8_t v00, v01, v10, v11;

	i0 = 0;
	j0 = 0;

	I = nrows;
	J = ncols;
	for (i = 0; i<di; ++i)
	{
		for (j = 0; j<dj; ++j)
		{
			i0 = i*I / di;
			j0 = j*J / dj;

			i1 = (i + 1)*I / di;
			j1 = (j + 1)*J / dj;

			if (i1 >= I)
				i1 = I - 1;

			if (j1 >= J)
				j1 = J - 1;

			rc_clahem(imaps[i][j], in, i0, j0, i1, j1, ldim, s);

			ics[i] = (i0 + i1) / 2;
			jcs[j] = (j0 + j1) / 2;
		}
	}

	/* SPECIAL CASE: image corners */
	for (i = 0; i<ics[0]; ++i)
	{
		for (j = 0; j<jcs[0]; ++j)
			out[i*ldim + j] = imaps[0][0][in[i*ldim + j]];

		for (j = jcs[dj - 1]; j<J; ++j)
			out[i*ldim + j] = imaps[0][dj - 1][in[i*ldim + j]];
	}

	for (i = ics[di - 1]; i<I; ++i)
	{
		for (j = 0; j<jcs[0]; ++j)
			out[i*ldim + j] = imaps[di - 1][0][in[i*ldim + j]];

		for (j = jcs[dj - 1]; j<J; ++j)
			out[i*ldim + j] = imaps[di - 1][dj - 1][in[i*ldim + j]];
	}

	/* SPECIAL CASE: image boundaries */
	for (k = 0; k<di - 1; ++k)
	{
		for (i = ics[k]; i<ics[k + 1]; ++i)
		{
			for (j = 0; j<jcs[0]; ++j)
			{
				v00 = imaps[k + 0][0][in[i*ldim + j]];
				v10 = imaps[k + 1][0][in[i*ldim + j]];

				out[i*ldim + j] = ((ics[k + 1] - i)*v00 + (i - ics[k])*v10) / (ics[k + 1] - ics[k]);
			}

			for (j = jcs[dj - 1]; j<J; ++j)
			{
				v01 = imaps[k + 0][dj - 1][in[i*ldim + j]];
				v11 = imaps[k + 1][dj - 1][in[i*ldim + j]];

				out[i*ldim + j] = ((ics[k + 1] - i)*v01 + (i - ics[k])*v11) / (ics[k + 1] - ics[k]);
			}
		}
	}

	for (k = 0; k<dj - 1; ++k)
		for (j = jcs[k]; j<jcs[k + 1]; ++j)
		{
			for (i = 0; i<ics[0]; ++i)
			{
				v00 = imaps[0][k + 0][in[i*ldim + j]];
				v01 = imaps[0][k + 1][in[i*ldim + j]];

				out[i*ldim + j] = ((jcs[k + 1] - j)*v00 + (j - jcs[k])*v01) / (jcs[k + 1] - jcs[k]);
			}

			for (i = ics[di - 1]; i<I; ++i)
			{
				v10 = imaps[di - 1][k + 0][in[i*ldim + j]];
				v11 = imaps[di - 1][k + 1][in[i*ldim + j]];

				out[i*ldim + j] = ((jcs[k + 1] - j)*v10 + (j - jcs[k])*v11) / (jcs[k + 1] - jcs[k]);
			}
		}
	for (k = 0; k<di - 1; ++k)
		for (l = 0; l<dj - 1; ++l)
			for (j = jcs[l]; j<jcs[l + 1]; ++j)
				for (i = ics[k]; i < ics[k + 1]; ++i)
				{
					uint8_t p;

					p = in[i*ldim + j];

					v00 = imaps[k + 0][l + 0][p];
					v01 = imaps[k + 0][l + 1][p];
					v10 = imaps[k + 1][l + 0][p];
					v11 = imaps[k + 1][l + 1][p];
					out[i*ldim + j] =
						(
						(ics[k + 1] - i)*(jcs[l + 1] - j)*v00 + (ics[k + 1] - i)*(j - jcs[l])*v01 + (i - ics[k])*(jcs[l + 1] - j)*v10 + (i - ics[k])*(j - jcs[l])*v11
							) / ((ics[k + 1] - ics[k])*(jcs[l + 1] - jcs[l]));
				}
}
/*
* Portion based on the work of Nenad Markus n3ar. 
*/
 void transform_to_ascii(ascii_render *pRender, uint8_t pixels[], int* nrows, int* ncols, unsigned char *zBuf)
{
	uint8_t *zPtr = zBuf;
	int ldim = *ncols;
	uint8_t* glyph;
	int i, j, idx;
	int r, c, n;
	
	*nrows = (*nrows / pRender->nRows) * pRender->nRows;
	*ncols = (*ncols / pRender->nCols) * pRender->nCols;

	compute_index_matrix(pRender, pixels, *nrows, *ncols, ldim);
	n = 0;
	for (r = 0; r < *nrows; r += pRender->nRows) {
		for (c = 0; c < *ncols; c += pRender->nCols) {
			idx = pRender->zMatrix[n];
			++n;
			glyph = (uint8_t*)&pRender->zGlyphs[idx][0];
			if (zPtr) *zPtr++ = glyph_char_table[idx];
			for (i = 0; i < pRender->nRows; ++i)
				for (j = 0; j < pRender->nCols; ++j)
					pixels[(r + i)*ldim + (c + j)] = glyph[i*pRender->nCols + j];
		}
		if (zPtr) *zPtr++ = '\n';
	}
}

/*
* CAPIREF: Refer to the official documentation for the main purpose of this interface.
*/
void AsciiArtInit(ascii_render *pRender)
{
 /*	memset(pRender, 0, sizeof(ascii_render));   */
	uint8_t *zBin_ptr = arr_to_ptr(zBin);
	count_t ct;
	//parse_art_model_(pRender->zGlyphs, &pRender->nGlyphs, &pRender->nRows, &pRender->nCols,&ct, &pRender->pTree,  zBin);
	parse_art_model_(pRender->zGlyphs, &pRender->nGlyphs, &pRender->nRows, &pRender->nCols, &ct, &pRender->pTree,  zBin_ptr);
	uint8_t pixel;
	//get_tree_output(pRender->pTree, &pixel, 2);
	int arr[2];
	//print_tree(pRender->pTree);
	//printf("%d\n", pRender->pTree[1]);
}
/*
* CAPIREF: Refer to the official documentation for the main purpose of this interface.
*/
unsigned int AsciiArtTextBufSize(ascii_render *pRender, int img_width, int img_height)
{
	return (img_height / ((pRender->nRows==0) ? pRender->nRows : 1)) * (img_width / ((pRender->nCols==0) ?  pRender->nCols + 1 : 1)) * sizeof(uint8_t);
}
/*
* CAPIREF: Refer to the official documentation for the main purpose of this interface.
*/
void AsciiArtRender(ascii_render *pRender, unsigned char *zPixel /*IN/OUT*/, int *pnWidth /*IN/OUT*/, int *pnHeight /*IN/OUT*/, unsigned char *zBuf/* Optional/OUT */, int Optimize)
{
	int ncol = *pnWidth;
	if (Optimize) {
		clahe_preprocess(zPixel, zPixel, *pnHeight, *pnWidth, 8, 8, 3);
	}
	transform_to_ascii(&(*pRender), zPixel, pnHeight, pnWidth, zBuf);
	if (*pnWidth < ncol) {
		/* Restore original width */
		*pnWidth = ncol;
	}
}
//#ifdef ART_ENABLE_STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
/*
* CAPIREF: Refer to the official documentation for the main purpose of this interface.
*/

char * AsciiArtLoadImage(const char * zPath, int * pWidth, int * pHeight)
{
	unsigned char *zBlob;
	int c;
	zBlob = stbi_load(zPath,pWidth, pHeight, &c, 1);
	return zBlob;
}

void getstbi(const char *fname) {
	int arr[101];
	const char *matrix = stbi_parse(fname);
	int x;
	if(matrix!=NULL) x = *matrix;
	for(int i=0,k=0;i<50;i++,k++) { 
	      arr[i] = matrix[i];
	}
	arr[101] = -1;
	if(check_zeros(arr)>10) printf("prolly got an png file or error in parsing\n");
}

/* int main() { */
/*     char path = "me.bmp"; */
/*     int* h; int* w ; */
/*     char* img = AsciiArtLoadImage(path, h, w); */
/*     return 0; */

/* } */

//#endif /* STB_IMAGE_IMPLEMENTATION */
