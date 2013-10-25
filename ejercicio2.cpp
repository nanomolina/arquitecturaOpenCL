#include <stdio.h>
#include <CL/cl.h>

#define N 2

int main()
{
    // Platform & Device
    cl_uint num_platforms;
    if(clGetPlatformIDs(0, NULL, &num_platforms) != CL_SUCCESS) {
        printf("Error!\n");
    } else {
        printf("Plataformas Disponibles: %u\n", num_platforms);
    }

    cl_platform_id platform_id;
    clGetPlatformIDs(1, &platform_id, &num_platforms);
    printf("Plataforma creada\n");

    cl_uint num_devices;
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    printf("Dispositivos Disponibles Total: %u\n", num_devices);

    cl_device_id *device;
    device = new cl_device_id[(int)num_devices];
    if (clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, num_devices, device, NULL) != 
    CL_SUCCESS) {
        printf("Error!\n");
    } else {
        printf("Dispositivos creados\n");
    }

    // cual usamos...
    int deviceUse = 1;

    char platformProfile[1000];
    clGetPlatformInfo(platform_id, CL_PLATFORM_PROFILE, 1000*sizeof(char), 
    &platformProfile, NULL);
    printf("Perfil de la plataforma: %s\n", platformProfile);


    char platformName[1000];
    clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 1000*sizeof(char), 
    &platformName, NULL);
    printf("Nombre de la plataforma: %s\n", platformName);

    char deviceName[1000];
    clGetDeviceInfo(device[deviceUse], CL_DEVICE_NAME, 1000*sizeof(char), 
    &deviceName, NULL);
    printf("Device name: %s\n", deviceName);

//--------------------------------Program------------------------------

    // Creating Context
    cl_context context;
    cl_int err;
    context = clCreateContext(NULL, 1, &device[deviceUse], NULL, NULL, &err);
    if(err == CL_SUCCESS) {
        printf("Contexto Creado\n");
    } else {
        printf("Error creando contexto\n");
    }


    // Program Objects
    const char* programSource =
    "__kernel                                   \n"
    "void vecadd(const int N,                   \n"
    "            __global float *A,             \n"
    "            __global float *B,             \n"
    "            __global float *C) {           \n"
    "   int x_id = get_global_id(0);            \n"
    "   int y_id = get_global_id(1);            \n"
    "   float temp;                             \n"
    "   temp = 0.0;                             \n"
    "                                           \n"
    "   for(int i=0; i<N ; i++){                \n"
    "      temp += A[x_id*N+i] * B[N*i+y_id];   \n"
    "   }                                       \n"
    "                                           \n"
    "   barrier(CLK_GLOBAL_MEM_FENCE);                                        \n"
    "   C[x_id*N+y_id] = temp;                  \n"
    "                                           \n"
    "}                                          \n"
    ;

    cl_program program;
    program = clCreateProgramWithSource(context, 1, 
    (const char **)&programSource, NULL, &err);
    if(err == CL_SUCCESS) {
        printf("Programa Creado\n");
    } else {
        printf("Error creando programa\n");
    }

    err = clBuildProgram(program, 1, &device[deviceUse], NULL, NULL, NULL);
    if(err == CL_SUCCESS) {
        printf("Programa compilado\n");
    } else {
        size_t len;
        printf("Error compilando programa\n");
        char buffer[2048];
        clGetProgramBuildInfo(program, device[deviceUse], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
    }

    cl_kernel kernel;
    kernel = clCreateKernel(program, "vecadd", &err);
    if(err == CL_SUCCESS) {
        printf("Kernel Creado\n");
    } else {
        printf("Error creando kernel\n");
    }
    // Memory Objects:
    float a_data[N*N], b_data[N*N], c_res[N*N];


    for (int i=0; i<N * N ; i++){
        a_data[i] = (float)(rand()*10.0/RAND_MAX);
        b_data[i] = (float)(rand()*10.0/RAND_MAX);
    }

    cl_mem a_in, b_in, c_out;

    a_in = clCreateBuffer(context, CL_MEM_READ_ONLY, N*N*sizeof(float),
    NULL, &err);
    if(err == CL_SUCCESS) {
        printf("Buffer A Creado\n");
    } else {
        printf("Error creando Buffer\n");
    }

    b_in = clCreateBuffer(context, CL_MEM_READ_ONLY, N*N*sizeof(float), 
    NULL, &err);
    if(err == CL_SUCCESS) {
        printf("Buffer B Creado\n");
    } else {
        printf("Error creando Buffer\n");
    }

    c_out = clCreateBuffer(context, CL_MEM_READ_WRITE, N*N*sizeof(float), 
    NULL, &err);
    if(err == CL_SUCCESS) {
        printf("Buffer C Creado\n");
    } else {
        printf("Error creando Buffer\n");
    }


    // Set Kernel Arguments

    int Ndim = N;
    err = 0;
    err = clSetKernelArg(kernel, 0, sizeof(int), &Ndim);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &a_in);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &b_in);
    err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &c_out);
    if(err == CL_SUCCESS) {
        printf("Argumentos del kernel configurados\n");
    } else {
        printf("Error configurando argumentos del kernel\n");
    }

    // Create Command Queue
    cl_command_queue commands;
    commands = clCreateCommandQueue(context, device[deviceUse], 0, &err);
    if(err == CL_SUCCESS) {
        printf("Cola de comandos creada\n");
    } else {
        printf("Error creando cola de comandos\n");
    }

    // Memory Write
    err = clEnqueueWriteBuffer(commands, a_in, CL_TRUE, 0, N*N*sizeof(float), 
    a_data, 0, NULL, NULL);
    if(err == CL_SUCCESS) {
        printf("Buffer A copiado a Device\n");
    } else {
        printf("Error copiando Buffer a Device\n");
    }

    err = clEnqueueWriteBuffer(commands, b_in, CL_TRUE, 0, N*N*sizeof(float), 
    b_data, 0, NULL, NULL);
    if(err == CL_SUCCESS) {
        printf("Buffer B copiado a Device\n");
    } else {
        printf("Error copiando Buffer a Device\n");
    }

    // Kernel Execution
    size_t global[2];
    global[0] = (size_t) N;
    global[1] = (size_t) N;
    err = clEnqueueNDRangeKernel(commands, kernel, 2, NULL, global, NULL, 0, 
    NULL, NULL);
    if(err == CL_SUCCESS) {
        printf("Kernel enviado a Device\n");
    } else {
        printf("Error enviando Kernel a Device\n");
    }

    clFinish(commands);

    err = clEnqueueReadBuffer(commands, c_out, CL_TRUE, 0, N*N*sizeof(float), 
    c_res, 0, NULL, NULL);
    if(err == CL_SUCCESS) {
        printf("Buffer C copiado desde Device\n");
    } else {
        printf("Error copiando Buffer desde Device\n");
    }

    printf("MATRIZ C:\n");
    for (int i=0; i<N*N; i++){
            printf("C[%i] = %f \n",i, c_res[i]);
    }
    delete device;
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseMemObject(a_in);
    clReleaseMemObject(b_in);
    clReleaseMemObject(c_out);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);
}
