// clang -framework OpenCL dumpcl.c -o dumpcl && ./dumpcl

/* ************************************************************
 INCLUDES / PREPROCESSOR / MODIFIERS
 ************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <OpenCL/opencl.h>
#include "kernel.cl.h"
#include <iostream>
#include "time.h"
#include <fstream>

using namespace std;

/* ************************************************************
 IFDEF
   ************************************************************ */

#define DEBUG
#undef DEBUG

#define PRINT_RANDOM_ARRAY
#undef	PRINT_RANDOM_ARRAY

/* ************************************************************
 VARIABLES
 ************************************************************ */

#define		NUM_VALUES			100
#define		DEFINED_WORKSIZE	10
#define		INPUT_SIZE			15

#define		height				15
#define		length				5000000
#define		GPU_THREAD_AMOUNT	length
#define		NUMBER_OF_RUNS		3

ofstream myfile;

struct mtsc_information
{
	int mtsc_thread;			// this may not need to be here anymore but im lazy and a hoarder
	int mtsc;
	int mtsc_actual_thread;
	int mtsc_run;
	
	int mtsc_seed[height];
	int mtsc_sort[height];
};

mtsc_information mtsc_info =
{
	0,
	0,
	0,
	0,
	{0},
	{0}
};

mtsc_information runsArray[NUMBER_OF_RUNS];

/* ************************************************************
 PROTOTYPES
 ************************************************************ */

void getDevices(void);
void performCL(void);
void performCL2(void);
int performCL_topswap(cl_int* input, int lininsize, int execution_amount);
int* linearize_array(int** array);
int* randomize_array(int* lin_array, int lapse, int lininsize);
int* randomize_array_kernel(int* lin_array, int lapse, int lininsize);

//MAKE A FILE TO TRACK HOW MANY FILES HAVE BEEN CREATED
//ALSO MAKE AN ARRAY THAT TRACKS EACH RUN THROUGH THEN LOOKS AT THE END, MAKE THIS OFF THE GLOBAL AMOUNT OF RUNS SPECIFIED


/*struct cl_image_information
{
	cl_image_format format;
	
	int width;
	int height;
	int depth;
	
	IOSurfaceRef iosurfaceref;
};

cl_image_information info = {CL_R, CL_UNSIGNED_INT32, 4, 4, 0, NULL};*/


// MAKE A RANDOM KERNEL THAT DIRECT PASSES


int main(int argc, char* const argv[])
{
	srand(time(NULL));
	//getDevices();
	//performCL();
	
	//performCL2();
	
	
	cl_int *input = (cl_int*)malloc(sizeof(cl_int) * INPUT_SIZE);
	
	int x = 0;
	int b = 0;
	int i = 0;
	myfile.open("myfile.txt");

	// maybe try randoming until you get some number of 0's
	
	/*for (int i = 0; i < INPUT_SIZE; i++)
	{
		input[i] = (i + 1);
	}
	for (int a = 0; a < INPUT_SIZE; a++)
	{
		for (int i = 0; i < INPUT_SIZE; i++)
		{
			printf("\n\nRandom Value: %d", x);
			
			
			printf("\nRun %d: \n", ((a * INPUT_SIZE) + i));
			for (int j = 0; j < INPUT_SIZE; j++)
			{
				printf("%d ", input[j]);
			}
			x = rand() % (INPUT_SIZE) + 1;
			int first = 0;
			int last = x - 1;
			int k = 0;
			while(first < last)
			{
				int temp = input[first];
				input[first] = input[last];
				input[last] = temp;
				first++;
				last--;
			}
			
		}
		b += i;
	 }
	
	//float* test_out = (float*)malloc(sizeof(cl_float) * NUM_VALUES);
	printf("\n\nFinal Randomized Array: \n");
	for (int j = 0; j < INPUT_SIZE; j++)
	{
		printf("%d ", input[j]);
		
	}*/
	//for (int i = 0; i < GPU_THREAD_AMOUNT; i++)
	{
		//performCL_topswap(input, INPUT_SIZE);
	}

	
	int **array;
	array = (int**)malloc(length * sizeof(int *));
	for (i = 0; i < length; i++)
	{
		array[i] = (int*)malloc(height * sizeof(int));
	}
	
	for (int x = 0; x < length; x++) // you can optimize this
	{
		int f = x % 2;
		for (int y = 0; y < height; y++)
		{
			if (f == 0)
			{
				array[x][y] = y + 1;
			}
			else
			{
				array[x][y] = height - y;
			}
		}
	}
	
	printf("\n\n\n\n");
	
	int* lin_array = linearize_array(array); // linearizing is done HORRAY 5 YEAR PLAN FINISHED IN 4. NOW MAKE THE KERNEL OFFSET AND STUFF
	
#ifdef	DEBUG
	printf("\n\n\nPrinting linearized array:\n\n");
	for (int n = 0; n < (height * length); n++)
	{
		printf("%d ", lin_array[n]);
	}
	printf("\n\n");
#endif	/* DEBUG */
	
	lin_array = randomize_array(lin_array, height, (height * length));
	
	
#ifdef	PRINT_RANDOM_ARRAY
	printf("\n\nPrinting randomized array: \n\n");
	for (int n = 0; n < (height * length); n++)
	{
		// check to see if we lapsed and newline that shit up yo
		if((n % height) == 0)
		{
			printf("\n");
		}
		printf("%d ", lin_array[n]);

	}
	printf("\n\n");
#endif	/* PRINT_RANDOM_ARRAY */
	int execution_amount = 0;
	int another = performCL_topswap(lin_array, (height * length), execution_amount);				// this will need to be remedied becasue first one does double random and linearizing ~~~
	execution_amount++;
	myfile.close();
	
	while(execution_amount < NUMBER_OF_RUNS)
	{
		myfile.open("myfile.txt", std::ios::app);
		myfile << execution_amount << "\n";
		printf("\n%d", execution_amount);
		lin_array = linearize_array(array);											// we could use another thread for this or possibly kernel these and thread them
		lin_array = randomize_array(lin_array, height, (height * length));		// sequential randomize
		//lin_array = randomize_array_kernel(lin_array, height, height * length);
		performCL_topswap(lin_array, (height * length), execution_amount);
		
		execution_amount++;
		
		myfile.close();
		
		//sleep(.2);			// not sure i need this anymore, it was an experiment
		
	}
	
	int max_mtsc = 0;
	int max_mtsc_thread = 0;
	
	for (int h = 0; h < NUMBER_OF_RUNS; h++)
	{
		if(runsArray[h].mtsc > max_mtsc)
		{
			max_mtsc = runsArray[h].mtsc;
			max_mtsc_thread = h;
		}
	}
	myfile.open("myfile.txt", std::ios::app);
	myfile << " ***************************  MAX_MTSC_INFO  *************************** \n\n" << "Max MTSC Thread: " << max_mtsc_thread << "\nMax MTSC Value: " << max_mtsc;
	printf(" ***************************  MAX_MTSC_INFO  *************************** \n\n");
	printf("Max MTSC Thread: %d\n", max_mtsc_thread);
	printf("Max MTSC Value: %d", max_mtsc);
	
	myfile << "\nSeed: ";
	printf("\nSeed: ");
	
	for(int h = 0; h < height; h++)
	{
		printf("%d ", runsArray[max_mtsc_thread].mtsc_seed[h]);
		myfile << runsArray[max_mtsc_thread].mtsc_seed[h] << " ";
	}
	
	myfile << "\nSort: ";
	printf("\nSort: ");
	
	for(int h = 0; h < height; h++)
	{
		printf("%d ", runsArray[max_mtsc_thread].mtsc_sort[h]);
		myfile << runsArray[max_mtsc_thread].mtsc_sort[h] << " ";
	}
	
	myfile << "\n\n\n\n";
	printf("\n\n\n\n");
	
	/*int j = 0;
	
	while(input[0] != 1)
	{
		int first = 0;
		int last = input[0] - 1;
		
		while(first < last)
		{
			int temp = input[first];
			input[first] = input[last];
			input[last] = temp;
			first++;
			last--;
		}
		j++;
		
		printf("Run %d: ", j);
		for (int i = 0; i < (sizeof(input) / sizeof(int)); i++)
		{
			printf("%d ", input[i]);
		}
		printf("\n\n");
	}*/
}


/*
 This function will list all OpenCL programmable devices contained in your system
 */

void getDevices()
{
	cl_uint num_devices, i;
	clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
	
	cl_device_id* devices = (cl_device_id*) calloc(sizeof(cl_device_id), num_devices);
	
	clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
	
	char name[128];
	for (i = 0; i < num_devices; i++)
	{
		clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 128, name, NULL);
		fprintf(stdout, "Device %s supports ", name);
		
		clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, 128, name, NULL);
		fprintf(stdout, "%s\n", name);
	}
	
	free(devices);
}

/*
 This function will perform whatever you want it to do based on the CL kernel that you have
 kernel.cl.
 
 THIS NEEDS TO BE MODIFED ON EVERY PROGRAM; IT IS NOT MAGIC.
 */

void performCL()
 {
 char name[128];
 int i = 0;
 
 dispatch_queue_t queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);
 
 cl_device_id device = gcl_get_device_id_with_dispatch_queue(queue);
 
 clGetDeviceInfo(device, CL_DEVICE_NAME, 128, name, NULL);
 fprintf(stdout, "Created a dispatch queue using the %s\n", name);
 
 float* test_in = (float*)malloc(sizeof(cl_float) * NUM_VALUES);
 for(i = 0; i < NUM_VALUES; i++)
 {
 test_in[i] = (cl_float)i;
 printf("%.5f  ", test_in[i]);
 }
 
 printf("\n\n\n\n\n\n");
 
 float* test_out = (float*)malloc(sizeof(cl_float) * NUM_VALUES);
 
 void* mem_in  = gcl_malloc(sizeof(cl_float) * NUM_VALUES, test_in, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
 
 void* mem_out = gcl_malloc(sizeof(cl_float) * NUM_VALUES, NULL, CL_MEM_WRITE_ONLY);
 
 dispatch_sync(queue,
 ^{
 size_t wgs;
 // Use CL_KERNEL_WORK_GROUP_SIZE to auto determine the workgroup size
 gcl_get_kernel_block_workgroup_info(square_kernel, DEFINED_WORKSIZE, sizeof(wgs), &wgs, NULL);
 cl_ndrange range =
 {
 1,
 {0, 0, 0},
 {NUM_VALUES, 0, 0},
 {wgs, 0, 0}
 };
 square_kernel(&range,(cl_float*)mem_in, (cl_float*)mem_out);
 gcl_memcpy(test_out, mem_out, sizeof(cl_float) * NUM_VALUES);
 });
 
 for(i = 0; i < NUM_VALUES; i++)
 {
 printf("%f  ", test_out[i]);
 }
 
 gcl_free(mem_in);
 gcl_free(mem_out);
 
 free(test_in);
 free(test_out);
 
 dispatch_release(queue);
 }

void performCL2()
{
	
	dispatch_queue_t dq = gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);
	
	
	
	char name[128];
	//int i = 0;
	
	//dispatch_semaphore_t dsema = dispatch_semaphore_create(0);
	//dispatch_queue_t queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);
	
	cl_device_id device = gcl_get_device_id_with_dispatch_queue(dq);
	
	clGetDeviceInfo(device, CL_DEVICE_NAME, 128, name, NULL);
	
	printf("Created dispatch queue with device: %s", name);
	
	cl_image_format image_format =
	{
		CL_R,
		CL_SIGNED_INT32
	};
	
	size_t cl_image_width = INPUT_SIZE;
	size_t cl_image_height = 1;
	size_t cl_image_depth	= 1;
	IOSurfaceRef iosurfaceref = NULL;
	
	cl_image in_image = gcl_create_image(&image_format, cl_image_width, cl_image_height, cl_image_depth, NULL);
	cl_image out_image = gcl_create_image(&image_format, cl_image_width, cl_image_height, cl_image_depth, NULL);
	
	unsigned int *pixels = (unsigned int*)malloc( sizeof(unsigned int) * cl_image_width * cl_image_height * cl_image_depth);
	
	/*for(int i = 0; i < (cl_image_width * cl_image_height * cl_image_depth); i++)
	{
		pixels[i] = (int)(cl_image_width * cl_image_height * cl_image_depth);
	}*/
	
	
	dispatch_sync(dq,
				  ^{
					  
					  size_t wgs;
					  gcl_get_kernel_block_workgroup_info(array_manip_kernel, 1000, sizeof(wgs), &wgs, NULL);
					  
					  // Use CL_KERNEL_WORK_GROUP_SIZE to auto determine the workgroup size
					  cl_ndrange range =
					  {
						  2,
						  {0},
						  //{cl_image_width, cl_image_height, cl_image_depth},
						  {1, 0, 0},
						  {wgs, 0, 0}
					  };
					  
					  
					  const size_t origin[3] = { 0, 0, 0 };
					  const size_t region[3] = { cl_image_width, cl_image_height, cl_image_depth };
					  
					  gcl_copy_ptr_to_image(in_image, pixels, origin, region);
					  
					  array_manip_kernel(&range, in_image, out_image, cl_image_width, cl_image_width, cl_image_depth, INPUT_SIZE);
					  
					  gcl_copy_image_to_ptr(pixels, out_image, origin, region);
					  
					  //dispatch_semaphore_signal(dsema);
					  
				  });
	
	//dispatch_semaphore_wait(dsema, DISPATCH_TIME_FOREVER);
	
	int total = (int)(cl_image_depth * cl_image_width * cl_image_height);
	
	printf("\n\n\nFinal Array: \n");
	
	int j[100] = {0};
	int k = 0;
	
	for(int i = 0; i < total; i++)
	{
		//printf("%d ", pixels[i]);
		
		if(i == pixels[i])
		{
			//printf("\nDamn, that's preatty cool!\n");
			j[k] = pixels[i];
			k++;
		}
	}
	int delimiter = 1;
	
	/*printf("\n\n\nPrinting all equivalents: \n");
	for (k = 0; (k < 100 && delimiter == 1); k++)
	{
		if (j[k] != 0)
		{
			printf("%d ", j[k]);
		}
		else
		{
			delimiter = 0;
		}
	}*/
	
	clReleaseMemObject(in_image);
    clReleaseMemObject(out_image);
	
    free(pixels);

	dispatch_release(dq);
}

int performCL_topswap(cl_int* input, int lininsize, int execution_amount)
{
	/*for (int k = 0; k < 10; k++)
	{*/
	char name[128];
	
	dispatch_queue_t queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);
	
	cl_device_id device = gcl_get_device_id_with_dispatch_queue(queue);
	
	clGetDeviceInfo(device, CL_DEVICE_NAME, 128, name, NULL);
	fprintf(stdout, "\nCreated a dispatch queue using the %s\n", name);
	
		
	int* test_out = (cl_int*)malloc(sizeof(cl_int) * lininsize);
	int* tsc_out = (cl_int*)malloc(sizeof(cl_int) * GPU_THREAD_AMOUNT);
	
	void* mem_in  = gcl_malloc(sizeof(cl_float) * lininsize, input, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
	
	void* mem_out = gcl_malloc(sizeof(cl_float) * lininsize, NULL, CL_MEM_WRITE_ONLY);
					  // Use CL_KERNEL_WORK_GROUP_SIZE to auto determine the workgroup size
	
	
		//FIX THIS TAB SHIT AND THEN ALSO MAKE IT MALLOC AND SHIT AND HAVE THE CPU PRINT OUT THE SORTED INPUT FROM THE GPU KERNEL*/
	printf("\n\n");
	/*for (int l = 0; l < INPUT_SIZE; l++)
	{
		printf("%d ", input[l]);
	}*/
	
	dispatch_sync(queue,
				  ^{
					  size_t wgs;
					  // Use CL_KERNEL_WORK_GROUP_SIZE to auto determine the workgroup size
					  gcl_get_kernel_block_workgroup_info(top_swap_kernel, GPU_THREAD_AMOUNT, sizeof(wgs), &wgs, NULL);
					  cl_ndrange range =
					  {
						  1,
						  {0, 0, 0},			// USE THIS OFFSET - FIRST MAKE 2D ARRAY THNE PASS TO LINEARIZER
						  {GPU_THREAD_AMOUNT, 0, 0},
						  {wgs, 0, 0}
					  };
					  top_swap_kernel(&range, (cl_int*)mem_in, (cl_int*)mem_out, lininsize, height);
					  gcl_memcpy(test_out, mem_in, sizeof(cl_int) * lininsize);
					  gcl_memcpy(tsc_out, mem_out, sizeof(cl_int) * GPU_THREAD_AMOUNT);
				  });
	
	
	
	dispatch_release(queue);
	
#ifdef PRINT_ALL_END
	printf("\n");
	for (int i = 0; i < lininsize; i++)
	{	if((i % height) == 0)
	{
		printf("\n");
	}
		printf("%d ", test_out[i]);
	}
	printf("\n\n");
#endif	/* PRINT_ALL_END */
	int mtsc = 0;
	int mtsc_thread = 0;
	int mtsc_pointer = 0;
	for(int i = 0; i < GPU_THREAD_AMOUNT; i++)
	{
		if(tsc_out[i] > mtsc)
		{
			mtsc = tsc_out[i];
			mtsc_thread = i;
			mtsc_pointer = i * height;
		}
		//printf("\n\n\n\GPU_THREAD_ID: %d	|	TopSwap Count: %d", i, tsc_out[i]);
	}
	
	
	runsArray[execution_amount].mtsc = mtsc;
	runsArray[execution_amount].mtsc_actual_thread = mtsc_thread;
	runsArray[execution_amount].mtsc_run = execution_amount;
	
	printf("MTSC INFORMATION:\n\nGPU_THREAD_ID: %d	 |	 TopSwap Count: %d\nSeed: ", mtsc_thread, mtsc);
	myfile << "MTSC INFORMATION:\n\nGPU_THREAD_ID: " << mtsc_thread << "	 |	 TopSwap Count: " << mtsc << "\nSeed: ";
	int h = 0;
	for(int notused = mtsc_pointer; notused < mtsc_pointer + height; notused++)
	{
		printf("%d ", input[notused]);
		myfile << input[notused] << " ";
		runsArray[execution_amount].mtsc_seed[h] = input[notused];
		h++;
	}
	printf("\nSort: ");
	myfile << "\nSort: ";
	h = 0;
	for(int notused = mtsc_pointer; notused < mtsc_pointer + height; notused++)
	{
		printf("%d ", test_out[notused]);
		myfile << test_out[notused] << " ";
		runsArray[execution_amount].mtsc_sort[h] = test_out[notused];
		h++;
	}
	printf("\n\n");
	myfile << "\n\n";
	
	free(input);
	free(test_out);
	free(tsc_out);
	gcl_free(mem_in);
	gcl_free(mem_out);
	
	return 1;					//dont change this, the return value is used as a boolean compare value
}

int* linearize_array(int** array)
{
	// add automatize size shit later
	int* lin_array = (int*)malloc(sizeof(int*) * height * length);
	int n = 0;
#ifdef DEBUG
	printf("\n\nPrinting original 2D array: \n\n");
	for (int y = 0; y < height; y++) // you can optimize this
	{
		for (int x = 0; x < length; x++)
		{
			printf("%-4d ", array[x][y]);
		}
		printf("\n");
	}
#endif	/* DEBUG */
	
	for (int x = 0; x < length; x++) // you can optimize this
	{
		for (int y = 0; y < height; y++)
		{
			lin_array[n] = array[x][y];
			n++;
		}
	}
	
	return lin_array;
}

int* randomize_array(int* lin_array, int lapse, int lininsize)													// try to just make it loop in the kernel
{																												// that will aslo make it use less transfer time
	int x = 0;
	int b = 0;
	int i = 0;
	int j = 0;
	//int a = 1;	// temp int to hold error delete to see error
	int k = 0;
	while (k < lininsize)
	{
		for (int a = 0; a < lapse; a++)
		{
			for (int i = 0; i < lapse; i++)
			{
				/*printf("\n\nRandom Value: %d", x);
				
				
				printf("\nRun %d: \n", ((a * lapse) + i));
				for (int j = 0; j < lapse; j++)
				{
					printf("%d ", lin_array[j]);
				}*/
				x = rand() % (lapse) + 1;
				int first = 0 + k;
				int last = (x - 1) + k;
				while(first < last)
				{
					int temp = lin_array[first];
					lin_array[first] = lin_array[last];
					lin_array[last] = temp;
					first++;
					last--;
				}
				
			}
			b += i;
			k += lapse;
		}
	}
	
	return lin_array;
}

/*int* randomize_array_kernel(int* lin_array, int lapse, int lininsize)
{
	char name[128];
	
	dispatch_queue_t queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);
	
	cl_device_id device = gcl_get_device_id_with_dispatch_queue(queue);
	
	clGetDeviceInfo(device, CL_DEVICE_NAME, 128, name, NULL);
	fprintf(stdout, "\nCreated a dispatch queue using the %s\n", name);
	
	
	int* test_out = (cl_int*)malloc(sizeof(cl_int) * lininsize);
	int* tsc_out = (cl_int*)malloc(sizeof(cl_int) * GPU_THREAD_AMOUNT);
	
	void* mem_in  = gcl_malloc(sizeof(cl_float) * lininsize, input, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
	
	void* mem_out = gcl_malloc(sizeof(cl_float) * lininsize, NULL, CL_MEM_WRITE_ONLY);

	
	dispatch_sync(queue,
				  ^{
					  size_t wgs;
					  // Use CL_KERNEL_WORK_GROUP_SIZE to auto determine the workgroup size
					  gcl_get_kernel_block_workgroup_info(randomize_array_kernel, GPU_THREAD_AMOUNT, sizeof(wgs), &wgs, NULL);
					  cl_ndrange range =
					  {
						  1,
						  {0, 0, 0},			// USE THIS OFFSET - FIRST MAKE 2D ARRAY THNE PASS TO LINEARIZER
						  {GPU_THREAD_AMOUNT, 0, 0},
						  {wgs, 0, 0}
					  };
					  top_swap_kernel(&range, (cl_int*)mem_in, (cl_int*)mem_out, lininsize, height);
					  gcl_memcpy(test_out, mem_in, sizeof(cl_int) * lininsize);
					  gcl_memcpy(tsc_out, mem_out, sizeof(cl_int) * GPU_THREAD_AMOUNT);
				  });
				  
	dispatch_release(queue);
	
	return lin_array;
}*/
