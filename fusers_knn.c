#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
//#include <linux/time.h>


#define MAX_LENGTH_STRING 150
typedef struct {
    unsigned int itemId;
    double rating;
} Item;

typedef struct {
    Item* item;
    unsigned int itemCount;
    int itemCapacity;
} User;

typedef struct {
    double distance;
    unsigned int userId;
} UserDistance;

typedef struct {
    unsigned int movieId;
    char name[MAX_LENGTH_STRING];
} Movie;


typedef double (*FuncionDistancia)(int, int);

User* users;
int usersNums = 0;
Movie* movies;
int movieCount = 0;

int compareItems(const void *a, const void *b) {
    return ((const Item *)a)->itemId - ((const Item *)b)->itemId;
}

int readItems(const char* filename, const char* delimiter, int hasHeader, int itemIndex) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error al abrir el archivo.\n");
        return -1;
    }

    int existItemId = 0;
    char line[MAX_LENGTH_STRING];  // Línea temporal para leer datos
    char* token;

    // Si el archivo tiene encabezado, lo saltamos
    if (hasHeader) {
        fgets(line, sizeof(line), file);
    }

    // Contar cuántas líneas (películas) hay para asignar memoria, para dejar de hacer realloc
    while (fgets(line, sizeof(line), file)) {
        movieCount++;
    }

    // Regresar al inicio del archivo para leer los datos
    rewind(file);
    if (hasHeader) {
        fgets(line, sizeof(line), file);  // Saltar encabezado de nuevo
    }

    // Asignar memoria para las películas
    movies = (Movie*)malloc(movieCount * sizeof(Movie));

    int i = 0;
    while (fgets(line, sizeof(line), file) && i < movieCount) {
        // Obtener el primer token (idMovie)
        //printf("Línea leída: %s", line);
        token = strtok(line, delimiter);
        if (token != NULL) {
            movies[i].movieId = atoi(token);  // Convertir a entero

            if (movies[i].movieId == itemIndex) {
                existItemId = 1;  // Verificar si el id existe
            }

            //printf("movie: %d\n", movies[i].movieId);

            // Obtener el segundo token (Name)
            token = strtok(NULL, delimiter);
            if (token != NULL) {
                strcpy(movies[i].name, token);  // Copiar el nombre

                // Remover el salto de línea al final de la cadena
                movies[i].name[strcspn(movies[i].name, "\n")] = '\0';
            }
        }
        i++;
    }

    fclose(file);
    return existItemId;
}

int getFirstValueFromLastLine(const char *filename, const char *delimiter) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return -1; // o un valor de error apropiado
    }

    // Mover al final del archivo
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    if (fileSize == 0) {
        fclose(file);
        return -1; // Archivo vacío
    }

    // Leer hacia atrás hasta encontrar la última línea
    long position = fileSize - 1;
    while (position > 0 && fgetc(file) != '\n') {
        fseek(file, --position, SEEK_SET);
    }

    // Ahora estamos al inicio de la última línea
    // Leer el primer valor
    int firstValue = 0;
    char c;
    int isFirstValue = 1;

    while ((c = fgetc(file)) != EOF && c != '\n') {
        if (c == delimiter[0] && isFirstValue) {
            break; // Si es el delimitador y ya leí el primer valor, salimos
        }
        if (isFirstValue && c != delimiter[0]) {
            firstValue = firstValue * 10 + (c - '0'); // Construir el número
        }
        if (c == delimiter[0]) {
            isFirstValue = 0; // Cambiamos a no primer valor
        }
    }

    fclose(file);
    return firstValue; // Devolver el primer valor encontrado
}

void readData(const char* filename, const char* delimiter, int hasHeader, int sort) {

    // Para hacer un estimado de los usuarios y dejar de hacer reallocate.
    int estimatedUsers = getFirstValueFromLastLine(filename, delimiter);

    printf("estimatedUsers: %d\n", estimatedUsers);

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    int capacity = estimatedUsers+1;
    users = (User*)malloc((capacity) * sizeof(User));

    char line[50];
    char format[20];

    sprintf(format, "%%d%s%%d%s%%lf", delimiter, delimiter);

    if (hasHeader) {
        fgets(line, sizeof(line), file);
    }

    while (fgets(line, sizeof(line), file)) {
        unsigned int uid;
        unsigned int iid;
        double rating;

        sscanf(line, format, &uid, &iid, &rating);

        if (uid > usersNums) {
            /*if (uid >= capacity) {
                capacity *= 2;
                users = (User*)realloc(users, (capacity) * sizeof(User));
            }*/

            users[uid].itemCount = 0;
            users[uid].itemCapacity = 2;
            users[uid].item = (Item*)malloc(users[uid].itemCapacity * sizeof(Item));

            if (sort && usersNums > 0) {
                qsort(users[usersNums].item, users[usersNums].itemCount, sizeof(Item), compareItems);
            }

            usersNums++;
        }

        if (users[uid].itemCount == users[uid].itemCapacity) {
            users[uid].itemCapacity *= 2;
            users[uid].item = (Item*)realloc(users[uid].item, users[uid].itemCapacity * sizeof(Item));
        }

        users[uid].item[users[uid].itemCount].itemId = iid;
        users[uid].item[users[uid].itemCount].rating = rating;
        users[uid].itemCount++;

    }

    if (sort) {
        qsort(users[usersNums].item, users[usersNums].itemCount, sizeof(Item), compareItems);
    }

    fclose(file);
}

double hallarDistanciaManhattan(int usuario1, int usuario2) {
    double distanciaManhattan = 0.0;
    unsigned int i = 0, j = 0, n = 0;
    bool flag = false;

    while (i < users[usuario1].itemCount && j < users[usuario2].itemCount) {
        if (users[usuario1].item[i].itemId == users[usuario2].item[j].itemId) {
            distanciaManhattan += fabs(users[usuario1].item[i].rating - users[usuario2].item[j].rating);
            i++;
            j++;
            n++;
            flag = true;
        } else if (users[usuario1].item[i].itemId < users[usuario2].item[j].itemId) {
            i++;
        } else {
            j++;
        }
    }

    if (n < 5) return NAN;
    return flag ? distanciaManhattan: NAN;
}

double hallarDistanciaEuclidiana(int usuario1, int usuario2) {
    double distanciaEuclidiana = 0.0, difference;
    unsigned int i = 0, j = 0, n = 0;
    bool flag = false;

    while (i < users[usuario1].itemCount && j < users[usuario2].itemCount) {
        if (users[usuario1].item[i].itemId == users[usuario2].item[j].itemId) {
            //printf("Iguales U1: %d con puntaje %lf\n", users[usuario1].item[i].itemId, users[usuario1].item[i].rating);
            //printf("Iguales U2: %d\n con puntaje %lf\n", users[usuario2].item[j].itemId, users[usuario2].item[j].rating);
            difference = users[usuario1].item[i].rating - users[usuario2].item[j].rating;
            distanciaEuclidiana += difference * difference;
            i++;
            j++;
            n++;
            flag = true;
        } else if (users[usuario1].item[i].itemId < users[usuario2].item[j].itemId) {
            i++;
        } else {
            j++;
        }
    }

    if (n < 5) return NAN;

    return flag ? sqrt(distanciaEuclidiana): NAN;;
}

double hallarSimilitudCoseno(int usuario1, int usuario2) {
    double productoPunto = 0.0, magnitudUsuario1 = 0.0, magnitudUsuario2 = 0.0, calificacion1, calificacion2;
    unsigned int i = 0, j = 0, n = 0;

    while (i < users[usuario1].itemCount && j < users[usuario2].itemCount) {
        if (users[usuario1].item[i].itemId == users[usuario2].item[j].itemId) {

            calificacion1 = users[usuario1].item[i].rating;
            calificacion2 = users[usuario2].item[j].rating;

            productoPunto += calificacion1 * calificacion2;
            magnitudUsuario1 += calificacion1 * calificacion1;
            magnitudUsuario2 += calificacion2 * calificacion2;
            i++;
            j++;
            n++;
        } else if (users[usuario1].item[i].itemId < users[usuario2].item[j].itemId) {
            i++;
        } else {
            j++;
        }
    }

    if (n < 5) return NAN;

    return (magnitudUsuario1 > 0 && magnitudUsuario2 > 0) ? (productoPunto / (sqrt(magnitudUsuario1 * magnitudUsuario2))) : NAN;
}

double hallarCorrelacionPearsonMejorada(int usuario1, int usuario2) {
    double sumXY = 0.0, sumX = 0.0, sumY = 0.0, sumX2 = 0.0, sumY2 = 0.0, x, y;
    unsigned int n = 0, i = 0, j = 0;

    while (i < users[usuario1].itemCount && j < users[usuario2].itemCount) {
        if (users[usuario1].item[i].itemId == users[usuario2].item[j].itemId) {
            x = users[usuario1].item[i].rating;
            y = users[usuario2].item[j].rating;

            sumXY += x * y;
            sumX += x;
            sumY += y;
            sumX2 += x * x;
            sumY2 += y * y;
            n++;
            i++;
            j++;
        } else if (users[usuario1].item[i].itemId < users[usuario2].item[j].itemId) {
            i++;
        } else {
            j++;
        }
    }

    if (n < 5) return NAN;

    double numerador = sumXY - (sumX * sumY / n);
    double denominador = sqrt((sumX2 - (sumX * sumX / n)) * (sumY2 - (sumY * sumY / n)));
    return (denominador != 0) ? numerador / denominador : NAN;
}

int compararDistancia(const void* a, const void* b) {
    double diff = ((UserDistance*)a)->distance - ((UserDistance*)b)->distance;
    return (diff > 0) - (diff < 0);
}

void hallarKnn(int usuarioIndex, int algoritmo, int k) {

    UserDistance distancias[usersNums - 1];
    int count = 0;

    printf("%d vecinos cercanos a %d usando ",k, usuarioIndex);
    FuncionDistancia funcion = NULL;
    switch (algoritmo) {
        case 1:
            printf("Distancia Manhattan:\n");
            funcion = hallarDistanciaManhattan;
            break;
        case 2:
            printf("Distancia Euclidiana:\n");
            funcion = hallarDistanciaEuclidiana;
            break;
        case 3:
            printf("Similitud de Coseno:\n");
            funcion = hallarSimilitudCoseno;
            break;
        case 4:
            printf("Correlacion de Pearson:\n");
            funcion = hallarCorrelacionPearsonMejorada;
            break;
        default:
            printf("Algoritmo no reconocido\n");
            exit(1); // Sale si el algoritmo no es reconocido
    }

    for (int i = 1; i <= usersNums; i++) {
        if (i == usuarioIndex) {
            continue;
        }

        UserDistance du;
        du.distance = funcion(usuarioIndex, i);

        if (!isnan(du.distance)) {
            du.userId = i;
            distancias[(count)++] = du;
        }
    }

    qsort(distancias, count, sizeof(UserDistance), compararDistancia);

    UserDistance distanciaKnn[k];
    int index = 0;

    if (algoritmo == 1 || algoritmo == 2) {
        for (int i = 0; i < count && index < k; i++) {
            distanciaKnn[index++] = distancias[i];
        }
    } else {
        for (int i = count - 1; i >= 0 && index < k; i--) {
            distanciaKnn[index++] = distancias[i];
        }
    }

    for (int i = 0; i < index && i < k; i++) {
        printf("%d. u%d con distancia %.13f\n", i + 1, distanciaKnn[i].userId, distanciaKnn[i].distance);
    }

}

const char* buscarNombrePelicula(unsigned int movieId) {
    for (int i = 0; i < movieCount; i++) {
        if (movies[i].movieId == movieId) {
            return movies[i].name;
        }
    }
    return NULL; // Retorna NULL si no encuentra la película
}

void hallarKnnConProyeccion(int usuarioIndex, int algoritmo, int k, int idItem, double umbral) {

    UserDistance distancias[usersNums - 1];
    int count = 0;

    printf("%d vecinos cercanos a %d usando ",k, usuarioIndex);
    FuncionDistancia funcion = NULL;
    switch (algoritmo) {
        case 1:
            printf("Distancia Manhattan:\n");
            funcion = hallarDistanciaManhattan;
            break;
        case 2:
            printf("Distancia Euclidiana:\n");
            funcion = hallarDistanciaEuclidiana;
            break;
        case 3:
            printf("Similitud de Coseno:\n");
            funcion = hallarSimilitudCoseno;
            break;
        case 4:
            printf("Correlacion de Pearson:\n");
            funcion = hallarCorrelacionPearsonMejorada;
            break;
        default:
            printf("Algoritmo no reconocido\n");
            exit(1); // Sale si el algoritmo no es reconocido
    }

    // Aplicamos el algoritmo con todos los users.
    for (int i = 1; i <= usersNums; i++) {
        if (i == usuarioIndex) {
            continue;
        } else if (users[usuarioIndex].itemCount == 0) {
            printf("Usuario %d no encontrado\n", usuarioIndex);
            exit(1);
        }

        UserDistance du;
        du.distance = funcion(usuarioIndex, i);

        if (!isnan(du.distance)) {
            du.userId = i;
            distancias[(count)++] = du;
        }
    }

    // Ordenamos.
    qsort(distancias, count, sizeof(UserDistance), compararDistancia);


    //UserDistance distanciaKnn[k];
    UserDistance * distanciaKnn = (UserDistance*) malloc(k * sizeof(UserDistance));
    int index = 0;

    // Truncamos segun K y el Algoritmo.
    if (algoritmo == 1 || algoritmo == 2) {
        for (int i = 0; i < count && index < k; i++) {
            distanciaKnn[index++] = distancias[i];
        }
    } else {
        for (int i = count - 1; i >= 0 && index < k; i--) {
            distanciaKnn[index++] = distancias[i];
        }
    }

    // Leemos el dataset de movies de paso nos aseguramos que el id item exista.
    //if (readItems("../dataset/movies.csv", ",", 1, idItem) == 0) {
    if (readItems("../dataset/movies.csv", ",", 1, idItem) == 0) {
        printf("\nNo existe este item: %d\n", idItem);
        exit(1);
    }

    // Nos aseguramos que el usuario index NO haya calificado el itemId.
    User user0 = users[usuarioIndex];
    for (unsigned int i = 0; i < user0.itemCount; i++) {
        if (user0.item[i].itemId == idItem) {
            printf("ALERTA) El usuario index: %d, ya califico el id: %d, por ende no necesitamos proyectar \n", usuarioIndex, idItem);
            exit(1);
        }
    }

    //Hallamos la proyeccion de calificaion para el itemId ingresado.
    double sumaKnn = 0.0, ratingsValidos[k], distaciasValidas[k];
    int indexValidos = 0;
    for (int i = 0; i < index && i < k; i++) {
        printf("-> %d. u%d con distancia %.10lf\n", i + 1, distanciaKnn[i].userId, distanciaKnn[i].distance);
        User user = users[distanciaKnn[i].userId];
        double rating = -1;
        // Verificamos si los k vecinos calificaron el itemId ingresado.
        for (unsigned int j = 0; j < user.itemCount; j++) {
            if (user.item[j].itemId == idItem) {
                rating = user.item[j].rating;
                // Para que el vecino entre a la ecuacion tiene que tener una distancia mayor a Cero(Pearson y Coseno) y la calificación del vecino tiene que ser mayor al umbral.
                if (rating < umbral || distanciaKnn[i].distance < 0.0) {
                    printf("Calificacion del vecino: %lf\n", rating);
                    printf("-----> OJO) El usuario %d, con la calificacion %lf a el item %d no cumple con el umbral o con la distancia %lf\n", distanciaKnn[i].userId, rating, idItem, distanciaKnn[i].distance);
                } else {
                    ratingsValidos[indexValidos] = rating;
                    distaciasValidas[indexValidos] = distanciaKnn[i].distance;
                    indexValidos++;
                    sumaKnn += distanciaKnn[i].distance;
                    printf("--->SumaKNN: %.10lf\n", sumaKnn);
                }
                break;
            }
        }

        if (rating == -1) {
            printf("->OJO) El usuario %d no califico el item %d \n", distanciaKnn[i].userId, idItem);
        }

    }

    //Liberamos la memoria heap.
    free(distanciaKnn);


    double ratingProyectado = 0;

    // En caso de que la suma KNN sea 0.0 o sea lso vecinos sean perfectos se saca un promedio simple(sin influencia).
    if (sumaKnn == 0.0 && (algoritmo == 1 || algoritmo == 2)) { // para manhhhatan y coseno cuando la suma es 0.0 en ese caso son perfectamente similares asi que todos tienen el mismo peso.
        for (int i = 0; i < indexValidos; i++) {
            ratingProyectado += ratingsValidos[i];
        }

        ratingProyectado = ratingProyectado / indexValidos;
    } else {
        // Con influencia
        for (int i = 0; i < indexValidos; i++) {
            printf("Hols\n");
            ratingProyectado += ratingsValidos[i] * (distaciasValidas[i] / sumaKnn);
        }
    }

    if (ratingProyectado == 0) ratingProyectado = NAN;

    const char* nombrePelicula = buscarNombrePelicula(idItem);

    printf("\nCalificacion proyectada del usuario %d a el (%d)item %s:  %.10lf\n", usuarioIndex, idItem, nombrePelicula, ratingProyectado);


    //printf("\nrating: %lf, distancia: %lf\n", ratingsValidos[i], distaciasValidas[i]);

    // Casos especiales:
    // El usuario NO tendria que haber calificado el id movie (Listo)
    // Los demas k usuarios si tendria que haber calificado id movie. (LISTO)
    // NO cumpla con el umbral: tendriamos que quitarlo de la ecuacion.
    // Que ks tomamos par que la ecuacion no se corrompa.


    // Pseudocódigo:

    // Entradas:
    // Usuario principal.
    // K-nn con un algoritmo en especifico: Person
    // Una Musica que el usuario principal no haya calificado, pero si los l usuasrios primeros.
    // De esos K sacar el puntaje que le dieron a la música
    // Umbral.

    // Influencia: Es de acuerso a os resultados de pEarson en este caso:
    // -> Se suma todos los l valores y acorde a esot se saca el porcentaje.

    // Formula: Suma de la calificaion a la musica seleccionada por  su influencia.


    //Input extra: umbral, idItem, userId

}

void imprimirUsuarios() {
    printf("\nDatos leídos:\n");
    for (int i = 1; i <= usersNums; i++) {  // Iterar desde 1 (usuarios comienzan desde 1)
        printf("Usuario %d: - NumItems: %d\n", i, users[i].itemCount);
        for (int j = 0; j < users[i].itemCount; j++) {
            printf("  Película %d: id = %d, rating = %.2f\n",
                   j + 1,
                   users[i].item[j].itemId,
                   users[i].item[j].rating);
        }
    }
}

int main() {
    
    //path, patron, header, sort 
    //readData("../dataset/ratings10m.dat", "::", 0, 0);
    readData("../dataset/ratings.csv", ",", 1, 0);
    printf("Numero de usuarios: %d\n", usersNums);

    //imprimirUsuarios();

    int usuariotIndex = 604;
    int idItem = 72720;
    int algoritmo = 2;
    int umbral = 3;
    int k = 20;

    struct timespec start_time, end_time;

    //int u2 = 74;    
    //int u1 = 10000;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    hallarKnnConProyeccion(usuariotIndex, algoritmo, k, idItem, umbral);
    //printf("Distancia: %.13lf\n", hallarDistanciaManhattan(u1,u2)); 
    //printf("Distancia: %.13lf\n", hallarDistanciaEuclidiana(u1,u2));
    //printf("Distancia: %.13lf\n", hallarSimilitudCoseno(u1,u2));
    //printf("Distancia: %.13lf\n", hallarCorrelacionPearsonMejorada(u1,u2));
    //hallarKnn(usuariotIndex, algoritmo, k);
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long long total_nanoseconds = (end_time.tv_sec - start_time.tv_sec) * 1000000000L + (end_time.tv_nsec - start_time.tv_nsec);
    printf("Tiempo de ejecucion: %lld nanosegundos\n", total_nanoseconds);

    // Liberar memoria
    for (int i = 1; i <= usersNums; i++) {
        free(users[i].item);
    }

    free(users);
    free(movies);
    return EXIT_SUCCESS;
}