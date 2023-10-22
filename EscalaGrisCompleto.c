#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <png.h>

//Se abre el archivo y en caso de fallo, se muestra mensaje de error
void convertir_a_escala_de_grises(const char *nombre_entrada, const char *nombre_salida) {
    FILE *archivo_entrada = fopen(nombre_entrada, "rb");
    if (!archivo_entrada) {
        printf("No se pudo abrir el archivo de entrada: %s\n", nombre_entrada);
        return;
    }

    //Se obtiene la extensión del archivo y en caso de fallo, muestra error
    const char *extension = strrchr(nombre_entrada, '.');
    if (!extension) {
        printf("Extensión de archivo no válida.\n");
        fclose(archivo_entrada);
        return;
    }

    if (strcasecmp(extension, ".jpg") == 0 || strcasecmp(extension, ".jpeg") == 0) {
        //Se lee una imagen JPEG y convierte a escala de grises
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr jerr;

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, archivo_entrada);
        jpeg_read_header(&cinfo, TRUE);
        jpeg_start_decompress(&cinfo);

        int row_stride = cinfo.output_width * cinfo.output_components;
        JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
        unsigned char *imagen_gris = (unsigned char *)malloc(cinfo.output_width * cinfo.output_height);

        unsigned char *ptr = imagen_gris;
        while (cinfo.output_scanline < cinfo.output_height) {
            jpeg_read_scanlines(&cinfo, buffer, 1);
            for (int i = 0; i < cinfo.output_width; i++) {
                *ptr++ = (unsigned char)((*buffer)[i * 3] * 0.299 + (*buffer)[i * 3 + 1] * 0.587 + (*buffer)[i * 3 + 2] * 0.114);
            }
        }

        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(archivo_entrada);

        //Se guarda la imagen en escala de grises y muestra error en caso de fallo
        FILE *archivo_salida = fopen(nombre_salida, "wb");
        if (!archivo_salida) {
            printf("No se pudo abrir el archivo de salida: %s\n", nombre_salida);
            free(imagen_gris);
            return;
        }

        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr) {
            fclose(archivo_salida);
            free(imagen_gris);
            return;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_write_struct(&png_ptr, NULL);
            fclose(archivo_salida);
            free(imagen_gris);
            return;
        }

        if (setjmp(png_jmpbuf(png_ptr)) == 1) {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            fclose(archivo_salida);
            free(imagen_gris);
            return;
        }

        png_init_io(png_ptr, archivo_salida);
        png_set_IHDR(png_ptr, info_ptr, cinfo.output_width, cinfo.output_height, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);

        png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * cinfo.output_height);
        for (int i = 0; i < cinfo.output_height; i++) {
            row_pointers[i] = imagen_gris + i * cinfo.output_width;
        }

        png_write_image(png_ptr, row_pointers);
        png_write_end(png_ptr, NULL);

        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(archivo_salida);
        free(imagen_gris);
        free(row_pointers);
    } else if (strcasecmp(extension, ".png") == 0) {
        //Se lee una imagen PNG y se convierte a escala de grises
        png_bytep *row_pointers;
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png) {
            fclose(archivo_entrada);
            return;
        }

        png_infop info = png_create_info_struct(png);
        if (!info) {
            png_destroy_read_struct(&png, NULL, NULL);
            fclose(archivo_entrada);
            return;
        }

        if (setjmp(png_jmpbuf(png)) == 1) {
            png_destroy_read_struct(&png, &info, NULL);
            fclose(archivo_entrada);
            return;
        }

        png_init_io(png, archivo_entrada);
        png_read_info(png, info);

        int ancho = png_get_image_width(png, info);
        int alto = png_get_image_height(png, info);
        png_byte color_type = png_get_color_type(png, info);
        png_byte bit_depth = png_get_bit_depth(png, info);

        if (bit_depth == 16) {
            png_set_strip_16(png);
        }

        if (color_type == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(png);
        }

        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
            png_set_expand_gray_1_2_4_to_8(png);
        }

        if (png_get_valid(png, info, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(png);
        }

        if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE) {
            png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
        }

        if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
            png_set_gray_to_rgb(png);
        }

        png_read_update_info(png, info);

        row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * alto);
        for (int y = 0; y < alto; y++) {
            row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png, info));
        }

        png_read_image(png, row_pointers);

        //Se convierte a escala de grises y se guarda
        FILE *archivo_salida = fopen(nombre_salida, "wb");
                if (!archivo_salida) {
            printf("No se pudo abrir el archivo de salida: %s\n", nombre_salida);
            for (int y = 0; y < alto; y++) {
                free(row_pointers[y]);
            }
            free(row_pointers);
            fclose(archivo_entrada);
            return;
        }

        unsigned char *imagen_gris = (unsigned char *)malloc(ancho * alto);

        for (int y = 0; y < alto; y++) {
            png_bytep row = row_pointers[y];
            for (int x = 0; x < ancho; x++) {
                png_bytep px = &(row[x * 4]);
                //Se convierten los pixeles a escala de grises
                imagen_gris[y * ancho + x] = (unsigned char)(0.299 * px[0] + 0.587 * px[1] + 0.114 * px[2]);
            }
        }

        fclose(archivo_entrada);

        //Se guarda la imagen en escala de grises y condicion de error en caso de fallo
        png_structp png_output = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_output) {
            free(imagen_gris);
            return;
        }

        png_infop info_output = png_create_info_struct(png_output);
        if (!info_output) {
            png_destroy_write_struct(&png_output, &info_output);
            free(imagen_gris);
            return;
        }

        if (setjmp(png_jmpbuf(png_output)) == 1) {
            png_destroy_write_struct(&png_output, &info_output);
            free(imagen_gris);
            return;
        }

        archivo_salida = fopen(nombre_salida, "wb");
        if (!archivo_salida) {
            printf("No se pudo abrir el archivo de salida: %s\n", nombre_salida);
            png_destroy_write_struct(&png_output, &info_output);
            free(imagen_gris);
            return;
        }

        png_init_io(png_output, archivo_salida);
        png_set_IHDR(png_output, info_output, ancho, alto, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_output, info_output);

        png_bytep row_output = (png_bytep)malloc(png_get_rowbytes(png_output, info_output));
        for (int y = 0; y < alto; y++) {
            png_bytep row = row_output;
            for (int x = 0; x < ancho; x++) {
                row[x] = imagen_gris[y * ancho + x];
            }
            png_write_row(png_output, row);
        }

        png_write_end(png_output, NULL);
        png_destroy_write_struct(&png_output, &info_output);
        fclose(archivo_salida);

        //Se libera la memoria utilizada por row_pointers e imagen_gris
        for (int y = 0; y < alto; y++) {
            free(row_pointers[y]);
        }
        free(row_pointers);
        free(imagen_gris);
    } else {
        printf("Extensión de archivo no compatible: %s\n", extension);
        fclose(archivo_entrada);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s [input.png o input.jpg] [nombre_salida]\n", argv[0]);
        return 1;
    }

    const char *nombre_entrada = argv[1];
    const char *nombre_salida = argv[2];

    convertir_a_escala_de_grises(nombre_entrada, nombre_salida);

    return 0;
}

