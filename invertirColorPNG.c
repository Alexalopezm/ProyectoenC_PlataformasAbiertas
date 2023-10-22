#include <stdio.h>
#include <stdlib.h>
#include <png.h>
 
void invertPNGColors(const char* input_filename, const char* output_filename) {
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        fprintf(stderr, "No se pudo abrir el archivo de entrada.\n");
        return;
    }
 
    // Crear una estructura png_struct
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(input_file);
        fprintf(stderr, "Error al crear la estructura PNG.\n");
        return;
    }
 
    // Crear una estructura png_info
    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        fclose(input_file);
        fprintf(stderr, "Error al crear la estructura de información PNG.\n");
        return;
    }
 
    // Configurar la lectura de archivo
    png_init_io(png, input_file);
    png_read_info(png, info);
 
    // Obtener información de la imagen
    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);
 
    // Verificar que la imagen sea en formato RGBA de 8 bits
    if (color_type != PNG_COLOR_TYPE_RGBA || bit_depth != 8) {
        fprintf(stderr, "La imagen debe ser en formato RGBA de 8 bits.\n");
        png_destroy_read_struct(&png, &info, NULL);
        fclose(input_file);
        return;
    }
 
    // Leer la imagen en una matriz de píxeles
    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
    }
    png_read_image(png, row_pointers);
 
    // Invertir los colores de la imagen
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width * 4; x += 4) {
            row_pointers[y][x] = 255 - row_pointers[y][x];         // Red
            row_pointers[y][x + 1] = 255 - row_pointers[y][x + 1]; // Green
            row_pointers[y][x + 2] = 255 - row_pointers[y][x + 2]; // Blue
            // No invertimos el canal alfa (transparencia) en este ejemplo
        }
    }
 
    // Crear un archivo de salida
    FILE *output_file = fopen(output_filename, "wb");
    if (!output_file) {
        fprintf(stderr, "No se pudo crear el archivo de salida.\n");
        return;
    }
 
    // Crear una estructura png_struct para la escritura
    png_structp png_out = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_out) {
        fclose(output_file);
        fprintf(stderr, "Error al crear la estructura PNG de salida.\n");
        return;
    }
 
    // Configurar la escritura de archivo
    png_init_io(png_out, output_file);
    png_set_IHDR(png_out, info, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
 
    // Escribir la imagen invertida
    png_write_info(png_out, info);
    png_write_image(png_out, row_pointers);
    png_write_end(png_out, NULL);
 
    // Liberar memoria y cerrar archivos
    fclose(input_file);
    fclose(output_file);
    
    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);
 
    png_destroy_read_struct(&png, &info, NULL);
    png_destroy_write_struct(&png_out, NULL);
 
    printf("Imagen invertida y guardada en %s\n", output_filename);
}
 
int main(int argc, char *argv[]) {
    const char* input_filename = argv[1];
    const char* output_filename = argv[2];
 
    invertPNGColors(input_filename, output_filename);
 
    return 0;
}
