#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

void invertJPEGColors(const char* input_filename, const char* output_filename) {
FILE *input_file = fopen(input_filename, "rb");
if (!input_file) {
fprintf(stderr, "No se pudo abrir el archivo de entrada.\n");
return;
}

// Estructuras necesarias para trabajar con JPEG
struct jpeg_decompress_struct cinfo;
struct jpeg_error_mgr jerr;

// Configurar manejadores de errores
cinfo.err = jpeg_std_error(&jerr);
jpeg_create_decompress(&cinfo);

// Asociar el archivo de entrada con el manejador JPEG
jpeg_stdio_src(&cinfo, input_file);

// Leer la cabecera de la imagen
(void)jpeg_read_header(&cinfo, TRUE);

// Iniciar la decodificación
(void)jpeg_start_decompress(&cinfo);

// Obtener información de la imagen
int width = cinfo.output_width;
int height = cinfo.output_height;
int num_components = cinfo.output_components;

// Reservar memoria para almacenar los datos de la imagen
JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * height);
for (int i = 0; i < height; i++) {
buffer[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * width * num_components);
}

// Leer la imagen línea por línea
while (cinfo.output_scanline < height) {
jpeg_read_scanlines(&cinfo, buffer + cinfo.output_scanline, height - cinfo.output_scanline);
}

// Invertir los colores de la imagen
for (int i = 0; i < height; i++) {
for (int j = 0; j < width * num_components; j++) {
buffer[i][j] = 255 - buffer[i][j];
}
}

// Crear un nuevo archivo JPEG para guardar la imagen invertida
FILE *output_file = fopen(output_filename, "wb");
if (!output_file) {
fprintf(stderr, "No se pudo crear el archivo de salida.\n");
return;
}

// Configurar estructuras JPEG para escritura
struct jpeg_compress_struct cinfo_out;
struct jpeg_error_mgr jerr_out;

cinfo_out.err = jpeg_std_error(&jerr_out);
jpeg_create_compress(&cinfo_out);
jpeg_stdio_dest(&cinfo_out, output_file);

// Configurar parámetros de salida
cinfo_out.image_width = width;
cinfo_out.image_height = height;
cinfo_out.input_components = num_components;
cinfo_out.in_color_space = JCS_RGB;

// Iniciar la compresión
jpeg_set_defaults(&cinfo_out);
jpeg_start_compress(&cinfo_out, TRUE);

// Escribir la imagen invertida
while (cinfo_out.next_scanline < height) {
jpeg_write_scanlines(&cinfo_out, buffer + cinfo_out.next_scanline, height - cinfo_out.next_scanline);
}

// Finalizar la compresión
jpeg_finish_compress(&cinfo_out);

// Cerrar los archivos
fclose(input_file);
fclose(output_file);

// Liberar la memoria
for (int i = 0; i < height; i++) {
free(buffer[i]);
}
free(buffer);

// Liberar estructuras JPEG
jpeg_destroy_compress(&cinfo_out);
jpeg_finish_decompress(&cinfo);
jpeg_destroy_decompress(&cinfo);
}

int main(int argc, char *argv[]) {
const char* input_filename = argv[1];
const char* output_filename = argv[2];

invertJPEGColors(input_filename, output_filename);

printf("Imagen con los colores invertida y guardada en el archivo %s.\n", output_filename);

return 0;
}
