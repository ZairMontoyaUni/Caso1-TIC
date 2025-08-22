#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


// Estructuras para encabezados de BMP
#pragma pack(push, 1)
//Este pragma le dice al compilador que alinee los campos de las estructuras con un empaquetamiento de 1 byte, es decir, 
//sin agregar relleno (padding) entre los campos.
typedef struct {
    unsigned short bfType;      // 'BM'
    unsigned int  bfSize;      // Tama o del archivo
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    uint32_t bfOffBits;   // Offset hasta los datos de imagen
} BITMAPFILEHEADER;

typedef struct {
    unsigned int  biSize;       // Tama o de el encabezado
    unsigned int   biWidth;      // Ancho en p xeles
    unsigned int   biHeight;     // Alto en p xeles
    unsigned short biPlanes;     // Siempre 1
    unsigned short biBitCount;   // Bits por p xel (8 para paletado)
    unsigned int  biCompression;
    unsigned int  biSizeImage;  // Tama o de los datos de imagen
    unsigned int   biXPelsPerMeter;
    unsigned int   biYPelsPerMeter;
    unsigned int  biClrUsed;    // Colores usados
    unsigned int  biClrImportant;
} BITMAPDIBHEADER;

typedef struct {
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
} PALETARGB;
#pragma pack(pop)


// Funcion para convertir la paleta a escala de grises
/* void convertir_a_grises(PALETARGB* paleta, int num_colores) {
    int i;   
    int gris;

    for ( i = 0; i < num_colores; i++) {
        gris = (paleta[i].rgbRed +  paleta[i].rgbGreen + paleta[i].rgbBlue)/3;
        paleta[i].rgbRed = gris;
        paleta[i].rgbGreen = gris;
        paleta[i].rgbBlue = gris;
    }

} */


// Zair Samuel Montoya Bello, Sebastian 
void convertir_a_grises(PALETARGB* paleta, int num_colores) {
    __asm {
        mov ebx, 0              ; i = 0

    loop_start:
        mov esi, [ebp+12]       ; cargar num_colores
        cmp ebx, esi
        jge loop_end

        mov edi, [ebp+8]        ; cargar paleta
        lea eax, [edi + ebx*4]  ; eax = &paleta[i]

        ; cargar componentes
        mov dl, [eax + 0]       ; B
        mov ch, [eax + 1]       ; G
        mov cl, [eax + 2]       ; R

        ; suma en 32 bits
        xor eax, eax
        movzx eax, cl           ; eax = R
        movzx ecx, ch           ; ecx = G
        add eax, ecx
        movzx ecx, dl           ; ecx = B
        add eax, ecx

        ; dividir suma / 3
        xor edx, edx
        mov ecx, 3
        div ecx                 ; eax = gris

        ; guardar gris
        mov edi, [ebp+8]        ; recargar paleta
        mov [edi + ebx*4 + 0], al  ; B
        mov [edi + ebx*4 + 1], al  ; G
        mov [edi + ebx*4 + 2], al  ; R

        inc ebx
        jmp loop_start

    loop_end:
        
    }
}





//Modo de uso: bitmap <archivobmp>
// genera archivo salida.bmp
int main (int argc, char* argv[]) {
  
    if (argc!=2) {
        printf("Este programa requiere un archivo BMP de 256 como entrada.\n");
        printf("Ejemplo: ./bitmap archivo.bmp \n");
        return 1;
    }

    FILE* in = fopen(argv[1], "rb");
    if (!in) {
        printf("No se pudo abrir el archivo de entrada");
        return 1;
    }

    // Leer cabecera del archivo
    BITMAPFILEHEADER file_header;
    fread(&file_header, sizeof(BITMAPFILEHEADER), 1, in);

    // Validar que sea un BMP de 8 bits
    if (file_header.bfType != 0x4D42) {
        printf("No es un archivo BMP valido.\n");
        fclose(in);
        return 1;
    }

    // Leer cabecera de informaci n
    BITMAPDIBHEADER info_header;
    fread(&info_header, sizeof(BITMAPDIBHEADER), 1, in);

    if (info_header.biBitCount != 8) {
        printf("Este programa solo soporta BMPs de 8 bits (256 colores).\n");
        fclose(in);
        return 1;
    }

    // Leer paleta de colores (256 entradas)
    PALETARGB paleta[256];
    fread(paleta, sizeof(PALETARGB), 256, in);

    // Calcular tamagnoo de los datos de imagen (puede incluir padding por fila)
    int ancho = info_header.biWidth;
    int alto = info_header.biHeight;
    if (alto < 0) alto = alto * (-1);
    int bytes_por_fila = (ancho + 3) & ~3; // Relleno a m ltiplo de 4
    int tam_imagen = bytes_por_fila * alto;

    // Leer datos de la imagen
    uint8_t* datos = (uint8_t*)malloc(tam_imagen);
    if (!datos) {
        printf("No se pudo reservar memoria.\n");
        fclose(in);
        return 1;
    }

    fread(datos, 1, tam_imagen, in);
    fclose(in);

    // Convertir paleta a escala de grises
    convertir_a_grises(paleta, 256);

    // Escribir archivo de salida
    FILE* out = fopen("salida.bmp", "wb");
    if (!out) {
        perror("No se pudo crear el archivo de salida");
        free(datos);
        return 1;
    }

    fwrite(&file_header, sizeof(BITMAPFILEHEADER), 1, out);
    fwrite(&info_header, sizeof(BITMAPDIBHEADER), 1, out);
    fwrite(paleta, sizeof(PALETARGB), 256, out);
    fwrite(datos, 1, tam_imagen, out);

    fclose(out);
    free(datos);

    printf("Imagen convertida a escala de grises y guardada como 'salida.bmp'\n");
    return 0;
}

