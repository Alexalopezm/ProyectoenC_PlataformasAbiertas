#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <png.h>
#include <string.h>

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


// Funcion que invierte las imagenes en PNG
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
 
}
 


int main(int argc, char *argv[]) {
	if (argc != 3) {
        printf("Uso: %s [input.png o input.jpg] [nombre_salida.png o nombre_salida.jpg]\n", argv[0]);
        return 1;
    }

	const char* input_filename = argv[1];
	const char* output_filename = argv[2];
	
    // Verifica si el nombre de archivo de entrada termina con ".jpg" o ".png"
    if (strstr(input_filename, ".jpg") || strstr(input_filename, ".png")) {
        // Comprueba si termina con ".jpg"
        if (strstr(input_filename, ".jpg")) {
            invertJPEGColors(input_filename, output_filename);
        }
        // Comprueba si termina con ".png"
        if (strstr(input_filename, ".png")) {
            invertPNGColors(input_filename, output_filename);
        }
        printf("Imagen invertida y guardada en %s\n", output_filename);
    } else {
        printf("El nombre de la imagen debe tener la extensión .png o .jpg\n");
    }

	return 0;
	

}

