#include <SDL2/SDL.h>       // Incluye la biblioteca SDL
#include <SDL2/SDL_image.h> // Incluye la extensión SDL_image
#include <stdio.h>

// Función para convertir una imagen a escala de grises
void convertir_a_escala_de_grises(const char *nombre_entrada, const char *nombre_salida) {
    // Inicializa SDL para trabajar con gráficos
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL no pudo inicializarse. SDL_Error: %s\n", SDL_GetError());
        return;
    }

    // Carga la imagen de entrada
    SDL_Surface *imagen = IMG_Load(nombre_entrada);
    if (!imagen) {
        printf("No se pudo cargar la imagen de entrada. SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    // Bloquea la superficie de la imagen para acceder a los píxeles
    SDL_LockSurface(imagen);
    for (int y = 0; y < imagen->h; y++) {
        for (int x = 0; x < imagen->w; x++) {
            // Obtiene el color del píxel en formato ARGB
            Uint32 pixel = *((Uint32 *)imagen->pixels + y * imagen->w + x);
            Uint8 r, g, b;
            // Extrae los componentes rojo, verde y azul del color
            SDL_GetRGB(pixel, imagen->format, &r, &g, &b);
            // Calcula el valor de gris utilizando una ponderación
            Uint8 gris = 0.299 * r + 0.587 * g + 0.114 * b;
            // Crea un nuevo color en escala de grises
            *((Uint32 *)imagen->pixels + y * imagen->w + x) = SDL_MapRGB(imagen->format, gris, gris, gris);
        }
    }
    // Desbloquea la superficie de la imagen
    SDL_UnlockSurface(imagen);

    // Guarda la imagen en escala de grises en formato BMP
    if (SDL_SaveBMP(imagen, nombre_salida) != 0) {
        printf("No se pudo guardar la imagen en escala de grises. SDL_Error: %s\n", SDL_GetError());
    }

    // Libera la memoria utilizada por la imagen
    SDL_FreeSurface(imagen);

    // Cierra SDL
    SDL_Quit();
}

// Función principal
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s [input.png] [nombre_salida]\n", argv[0]);
        return 1;
    }

    const char *nombre_entrada = argv[1];
    const char *nombre_salida = argv[2];

    // Llama a la función para convertir la imagen a escala de grises
    convertir_a_escala_de_grises(nombre_entrada, nombre_salida);

    return 0;
}
