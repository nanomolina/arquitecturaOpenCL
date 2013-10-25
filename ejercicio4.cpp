#include <stdio.h>
#include <CL/cl.h>

#define LENGTH 100000
int main()
{
    // Platform & Device
    cl_uint num_platforms;
    if(clGetPlatformIDs(0, NULL, &num_platforms) != CL_SUCCESS) { //get return values
        printf("Error!\n");
    } else {
        printf("Plataformas Disponibles: %u\n", num_platforms);
    }

    cl_platform_id platform_id;
    clGetPlatformIDs(1, &platform_id, &num_platforms);
    printf("Plataforma creada\n");

    cl_uint num_devices;
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices); //cpu's, gpu's, dps
    printf("Dispositivos Disponibles Total: %u\n", num_devices);

    cl_device_id device;
    if (clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, 1, &device, NULL) != CL_SUCCESS) {
        printf("Error!\n");
    } else {
        printf("Dispositivo creado\n");
    }

    char platformProfile[1000];
    clGetPlatformInfo(platform_id, CL_PLATFORM_PROFILE, 1000*sizeof(char), &platformProfile, NULL);
    printf("Perfil de la plataforma: %s\n", platformProfile);

    char platformName[1000];
    clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 1000*sizeof(char), &platformName, NULL);
    printf("Nombre de la plataforma: %s\n", platformName);

    char deviceName[1000];
    clGetDeviceInfo(device, CL_DEVICE_NAME, 1000*sizeof(char), &deviceName, NULL);
    printf("Device name: %s\n", deviceName);

    // Creating Context
    cl_context context;
    cl_int err;
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if(err == CL_SUCCESS) {
        printf("Contexto Creado\n");
    } else {
        printf("Error creando contexto\n");
    }

    // Program Objects
    const char* programSource =
    "__kernel                                       \n"
    "void vecadd(__global float *C)                 \n"
    "{                                              \n"
    "   int tid=get_global_id(0);                   \n"
    "   float num_steps = 100000.0;                 \n"
    "   float step = 1.0 / num_steps;               \n"
    "   float x, pi, sum=0.0;                       \n"
    "                                               \n"
    "   x = ((float) tid + 0.5) * step;             \n"
    "   sum = sum + 4.0 / (float) (1.0 + x * x);    \n"
    "                                               \n"
    "   C[tid]= (float)(step * sum);                \n"
    "}                                              \n"
    ;

    cl_program program;
    program = clCreateProgramWithSource(context, 1, (const char **)&programSource, NULL, &err);
    if(err == CL_SUCCESS) {
        printf("Programa Creado\n");
    } else {
        printf("Error creando programa\n");
    }

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
        if(err == CL_SUCCESS) {
        printf("Programa compilado\n");
    } else {
        size_t len;
        printf("Error compilando programa\n");
        char buffer[2048];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
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
    float c_res[LENGTH];

    cl_mem c_out;
    c_out = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float)*LENGTH, NULL, &err);
    if(err == CL_SUCCESS) {
        printf("Buffer C Creado\n");
    } else {
        printf("Error creando Buffer\n");
    }

    // Set Kernel Arguments
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &c_out);
    if(err == CL_SUCCESS) {
        printf("Argumentos del kernel configurados\n");
    } else {
        printf("Error configurando argumentos del kernel\n");
    }

    // Create Command Queue
    cl_command_queue commands;
    commands = clCreateCommandQueue(context, device, 0, &err);
    if(err == CL_SUCCESS) {
        printf("Cola de comandos creada\n");
    } else {
        printf("Error creando cola de comandos\n");
    }

    // Kernel Execution
    size_t global = LENGTH;
    size_t local = LENGTH;
    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
    if(err == CL_SUCCESS) {
        printf("Kernel enviado a Device\n");
    } else {
        printf("Error enviando Kernel a Device\n");
    }

    clFinish(commands);

    err = clEnqueueReadBuffer(commands, c_out, CL_TRUE, 0, sizeof(float)*LENGTH, c_res, 0, NULL, NULL);
    if(err == CL_SUCCESS) {
        printf("Buffer C copiado desde Device\n");
    } else {
        printf("Error copiando Buffer desde Device\n");
    }

    float result = 0.0;
    for (int i=0; i<LENGTH; i++){
//        printf("C[%i]= %f\n", i, c_res[i]);
        result += c_res[i];
    }
    printf("pi = %f\n", result);
}
