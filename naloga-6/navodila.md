# Naloga 6

Paralelizirajte sekvenčno kodo za risanje madelbrotove množice in jo
prilagodite za izvajanje na GPE z uporabo OpenCL knjižnice.

> compile and run:
> ```  gcc -lOpenCL -Wdeprecated-declarations -O3 -o solution solution.cpp  ```

## Outputs

```
[naloga-6] gcc -lOpenCL -Wdeprecated-declarations -o solution solution.cpp && ./solution
solution.cpp: In function ‘int main()’:
solution.cpp:107:86: warning: ‘_cl_command_queue* clCreateCommandQueue(cl_context, cl_device_id, cl_command_queue_properties, cl_int*)’ is deprecated [-Wdeprecated-declarations]
 eue = clCreateCommandQueue(context, device_id[0], 0, &ret);
                                                          ^
In file included from solution.cpp:5:0:
/usr/include/CL/cl.h:1443:1: note: declared here
 clCreateCommandQueue(cl_context                     /* context */,
 ^~~~~~~~~~~~~~~~~~~~


veckratnik niti = 32
Time GPU: 0.423356056213379s
Time CPU: 275.900161981582642s
GPU is 652x faster
image size: 19200 x 10800

```

```
[naloga-6] gcc -lOpenCL -Wdeprecated-declarations -o solution solution.cpp && ./solution
solution.cpp: In function ‘int main()’:
solution.cpp:107:86: warning: ‘_cl_command_queue* clCreateCommandQueue(cl_context, cl_device_id, cl_command_queue_properties, cl_int*)’ is deprecated [-Wdeprecated-declarations]
 eue = clCreateCommandQueue(context, device_id[0], 0, &ret);
                                                          ^
In file included from solution.cpp:5:0:
/usr/include/CL/cl.h:1443:1: note: declared here
 clCreateCommandQueue(cl_context                     /* context */,
 ^~~~~~~~~~~~~~~~~~~~


veckratnik niti = 32
Time GPU: 0.004832983016968s
Time CPU: 2.759490013122559s
GPU is 571x faster
image size: 1920 x 1080

```


```
[naloga-6] gcc -lOpenCL -Wdeprecated-declarations -O3 -o solution solution.cpp && ./solution
solution.cpp: In function ‘int main()’:
solution.cpp:107:86: warning: ‘_cl_command_queue* clCreateCommandQueue(cl_context, cl_device_id, cl_command_queue_properties, cl_int*)’ is deprecated [-Wdeprecated-declarations]
  cl_command_queue command_queue = clCreateCommandQueue(context, device_id[0], 0, &ret);
                                                                                      ^
In file included from solution.cpp:5:0:
/usr/include/CL/cl.h:1443:1: note: declared here
 clCreateCommandQueue(cl_context                     /* context */,
 ^~~~~~~~~~~~~~~~~~~~


veckratnik niti = 32 
Time GPU: 0.004574060440063s
Time CPU: 1.427638053894043s
GPU is 312x faster
image size: 1920 x 1080

```

```
[naloga-6] gcc -lOpenCL -Wdeprecated-declarations -O3 -o solution solution.cpp && ./solution
solution.cpp: In function ‘int main()’:
solution.cpp:107:86: warning: ‘_cl_command_queue* clCreateCommandQueue(cl_context, cl_device_id, cl_command_queue_properties, cl_int*)’ is deprecated [-Wdeprecated-declarations]
  cl_command_queue command_queue = clCreateCommandQueue(context, device_id[0], 0, &ret);
                                                                                      ^
In file included from solution.cpp:5:0:
/usr/include/CL/cl.h:1443:1: note: declared here
 clCreateCommandQueue(cl_context                     /* context */,
 ^~~~~~~~~~~~~~~~~~~~


veckratnik niti = 32 
Time GPU: 0.423525094985962s
Time CPU: 142.664970874786377s
GPU is 337x faster
image size: 19200 x 10800

```

