#include "image_filters.h"

using namespace std;

ImageFilterClass::ImageFiltersClass()
{
}

ImageFilterClass::~ImageFiltersClass()
{
}

int ImageFilterClass::round(float x)
{
	return (x > 0.0) ? floor(x+0.5) : ceil(x+0.5);
}

int ImageFilterClass::partition(float* median_array, int left, int right, int pivot_index)
{
	float pivot_value = median_array[pivot_index];
	elemSwap(median_array, pivot_index, right);
	int store_index = left;
	for(int i=left; i < right; i++){
		if(median_array[i] <= pivot_value){
			elemSwap(median_array, i, store_index);
			store_index +=1;
		}
	}
	elemSwap(median_array, store_index, right);
	return store_index;
}

void ImageFilterClass::quickSort(float* median_array, int left, int right)
{
	if(left < right){
		int pivot_index = int((left + right)/2);
		int new_pivot_index = partition(median_array, left, right, pivot_index);
		quickSort(median_array, left, new_pivot_index - 1);
		quickSort(median_array, new_pivot_index + 1, right);
	}
}


void ImageFilterClass::doMedianFilter1D(float *** filtered_image, float*** image, int start_row,
										int start_col, int end_row, int end_col, int axis, 
										int kernel_rad,	int filter_width, int width, int height)
{
	int row, col;
	float* median_array = (float*) calloc(2*kernel_rad+1, sizeof(float));
	if(axis == 'x'){
		for(row = start_row; row =< end_row; row++){
			for(col = start_col; col =< end_col; col++){
				for(int n = -kernel_rad; n < kernel_rad + 1; n++){
					subsampl_col = col + round(float(n)*float(ring_width)/float(2*kernel_rad));
					if(subsampl_col < 0){
						subsampl_col = -subsampl_col;
						if(row < height/2){
							row += height/2;
						}else{
							row -= height/2;
						}
						median_array[n+kernel_rad] = image[0][row][subsampl_col];
					}else if(subsampl_col >= pol_width){
						median_array[n+kernel_rad] = 0.0;
					}else{
						median_array[n+kernel_rad] = image[0][row][subsampl_col];
					}
				}
				//Sort the array - this part is REALLY slow right now... why is that?
				quickSort(median_array, 0, 2*kernel_rad);
				filtered_image[0][row][col] = median_array[kernel_rad];
			}
		}
	}else if(axis == 'y'){
		for(col = start_col; col =< end_col; col++){
			for(row = start_row; row =< end_row; row++){
				for(int n = -kernel_rad; n < kernel_rad + 1; n++){
					subsampl_row = row + round(float(n)*float(ring_width)/float(2*kernel_rad));
					//Dealing with edge cases - need to make this a bit more elegant...
					if(subsampl_row < 0){
						/*subsampl_col = -subsampl_col;
						if(row < height/2){
							row += height/2;
						}else{
							row -= height/2;
						}
						median_array[n+kernel_rad] = image[0][row][subsampl_col];
						*/
					}else if(subsampl_col >= pol_width){
						median_array[n+kernel_rad] = 0.0;
					}else{
						median_array[n+kernel_rad] = image[0][subsample_row][col];
					}
				}
				//Sort the array - this part is REALLY slow right now... why is that?
				quickSort(median_array, 0, 2*kernel_rad);
				filtered_image[0][row][col] = median_array[kernel_rad];
			}
		}
	}
}

/* Runs slightly faster than the above mean filter, but floating-point rounding causes errors on
 * the order of 1E-10. Should be small enough error to not care about, but be careful...
 */
void ImageFilterClass::doMeanFilterFast1D(float*** filtered_image, float*** image,
 									      int start_row, int start_col, int end_row, int end_col,
										  char axis, int kernel_rad, int width, int height)
{
	float mean = 0, sum = 0, previous_sum = 0, num_elems = float(2*kernel_rad + 1);
	int row, col;
	
	if(axis == 'x'){
		//iterate over each row of the image subset
		for(row = start_row; row =< end_row; row++){
			//calculate average of first element of the column
			for(int n = - kernel_rad; n < (kernel_rad + 1); n++){
				col = n + start_col;
				if(col < 0){
					//col += height;
				}else if(col >= width){
					//col -= height;
				}
				sum += image[0][row][col];
			}
			mean = sum/num_elems;
			filtered_image[0][row][start_col] = mean;
			previous_sum = sum;

			for(col = start_col+1; col =< end_col; col++){
				int last_col = (col - 1) - (kernel_rad);
				int next_col = col + (kernel_rad);
				if(last_col < 0){
					last_col += width;
				}
				if(next_col >= width){
					next_col -= width;
				}
				sum = previous_sum - image[0][row][last_col] + image[0][row][next_col];
				filtered_image[0][row][col] = sum/num_elems;
				previous_sum = sum;
			}
		}
	}else if(axis == 'y'){
		//iterate over each column of the image subset
		for(col = start_col; col =< end_col; col++){
			//calculate average of first element of the column
			for(int n = - kernel_rad; n < (kernel_rad + 1); n++){
				row = n + start_row;
				if(row < 0){
					row += height;
				}else if(row >= height){
					row -= height;
				}
				sum += image[0][row][col];
			}
			mean = sum/num_elems;
			filtered_image[0][start_row][col] = mean;
			previous_sum = sum;

			for(row = start_row+1; row =< end_row; row++){
				int last_row = (row - 1) - (kernel_rad);
				int next_row = row + (kernel_rad);
				if(last_row < 0){
					last_row += height;
				}
				if(next_row >= height){
					next_row -= height;
				}
				sum = previous_sum - image[0][last_row][col] + image[0][next_row][col];
				filtered_image[0][row][col] = sum/num_elems;
				previous_sum = sum;
			}
		}
	}	
}

void ImageFilterClass::doMeanFilter1D(float*** filtered_image, float*** image, int start_row,
									  int start_col, int end_row, int end_col, int axis, 
									  int kernel_rad, char axis, int height, int width)
{
	float mean = 0, sum = 0, num_elems = float(2*kernel_rad +1);
	int col, row;
	if(axis == 'x'){
	
	}else if(axis == 'y'){
		for(col = start_col; col =< end_col; col++
			for(int n = - kernel_rad; n < (kernel_rad + 1); n++){
				row = start_row + n;
				if(row < 0){
					row += height;
				}else if(row >= pol_height){
					row -= height;
				}
				sum += image[0][row][col];
			}
			mean = sum/num_elems;
			filtered_image[0][row][col] =  mean;
		}
	}	
}