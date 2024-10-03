//Notas:

//Ver el temas de los ids de las peliculas, que pasa si no estan en orden por ejemplo existen:
//IDs: 1, 2, 3, 5, 6, 10, como vemos no todas las peliculas pueden estar presentes...
//Tambien ver una manera más optima de manera los promedio que solo me sirven para el coseno
//Optimizar y probar con data mas grande;
//Analizar el tema de los denominadores... Evaluar el uso de hash

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

typedef struct {
    unsigned int itemId;
    double rating;
} Item;

typedef struct {
    Item* item;
    unsigned int itemCount;
    int itemCapacity;
    double average;
} User;

typedef struct {
    int itemId1;
    int itemId2;
    double CosenoAjustado;

} MatrizCoseno;

typedef struct {
    unsigned int movieId;
    char name[50];
} Movie;

User* users;
MatrizCoseno* matrizC;
Movie* movies;

int usersNums = 0;
int maxItems = 0;
int movieCount = 0;

int readItems(const char* filename, const char* delimiter, int hasHeader, int itemIndex) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error al abrir el archivo.\n");
        return -1;
    }

    int existItemId = 0;
    char line[100];  // Línea temporal para leer datos
    char* token;

    // Si el archivo tiene encabezado, lo saltamos
    if (hasHeader) {
        fgets(line, sizeof(line), file);
    }

    // Contar cuántas líneas (películas) hay para asignar memoria
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
        printf("Línea leída: %s", line);
        token = strtok(line, delimiter);
        if (token != NULL) {
            movies[i].movieId = atoi(token);  // Convertir a entero

            if (movies[i].movieId == itemIndex) {
                existItemId = 1;  // Verificar si el id existe
            }

            printf("movie: %d\n", movies[i].movieId);

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

int compareItems(const void *a, const void *b) {
    return ((const Item *)a)->itemId - ((const Item *)b)->itemId;
}

void readData(const char* filename, const char* delimiter, int hasHeader, int sort) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    int capacity = 100;
    double counterItemsRating = 0.0;
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
            if (uid >= capacity) {
                capacity *= 2;
                users = (User*)realloc(users, (capacity) * sizeof(User));
            }

            users[uid].itemCount = 0;
            users[uid].itemCapacity = 2;
            users[uid].item = (Item*)malloc(users[uid].itemCapacity * sizeof(Item));

            if (sort && usersNums > 0) {
                qsort(users[usersNums].item, users[usersNums].itemCount, sizeof(Item), compareItems);
            }

            if (users[usersNums].itemCount > maxItems) {
                maxItems = users[usersNums].itemCount;
            }

            users[usersNums].average = counterItemsRating / users[usersNums].itemCount;
            counterItemsRating = 0.0;
            usersNums++;
        }

        if (users[uid].itemCount == users[uid].itemCapacity) {
            users[uid].itemCapacity *= 2;
            users[uid].item = (Item*)realloc(users[uid].item, users[uid].itemCapacity * sizeof(Item));
        }

        users[uid].item[users[uid].itemCount].itemId = iid;
        users[uid].item[users[uid].itemCount].rating = rating;
        users[uid].itemCount++;
        counterItemsRating += rating;

    }

    if (sort) {
        qsort(users[usersNums].item, users[usersNums].itemCount, sizeof(Item), compareItems);
    }

    users[usersNums].average = counterItemsRating / users[usersNums].itemCount;
    
    if (users[usersNums].itemCount > maxItems) {
        maxItems = users[usersNums].itemCount;
    }

    fclose(file);
}

/*
double encontrarRatingSegunItemId(int userIndex, int idItem) {
    for (int i = 0; i < users[userIndex].itemCount + 1; i++) {
        if ((users[userIndex].item[i].itemId) == idItem) {
            return users[userIndex].item[i].rating;
        }
    }

    return -1.0;
}
*/

// Ahora se hace el caluclo en la lectura.
double calcularPromedioUsuario(int userIndex) {
    double sumaRatings = 0;
    for (int j = 0; j < users[userIndex].itemCount; j++) {
        sumaRatings += users[userIndex].item[j].rating;
    }
    return sumaRatings / users[userIndex].itemCount;
}

double hallarSimilitudDeCosenoAjustado(int item1, int item2) {
    double sumaNumerador = 0;
    double sumaDenominador1 = 0;
    double sumaDenominador2 = 0;
    //printf("\tITEMS: %d, %d\n", item1, item2);
    for (int i = 1; i <= usersNums; i++) {
        double ratingItem1 = 0;
        double ratingItem2 = 0;
        int haCalificadoItem1 = 0;
        int haCalificadoItem2 = 0;

        for (int j = 0; j < users[i].itemCount; j++) {
            if (users[i].item[j].itemId == item1) {
                //printf("item1: %d\n", item1);
                ratingItem1 = users[i].item[j].rating;
                //printf("ranking1: %lf\n", ratingItem1);
                haCalificadoItem1 = 1;
            }
            if (users[i].item[j].itemId == item2) {
                //printf("item2: %d\n", item2);
                ratingItem2 = users[i].item[j].rating;
                //printf("ranking2: %lf\n", ratingItem2);
                haCalificadoItem2 = 1;
            }

            if (haCalificadoItem1 && haCalificadoItem2) {
                break;
            }

        }

        if (haCalificadoItem1 && haCalificadoItem2) {

            //printf("->>>>>>User in: %d with average: %lf\n", i, users[i].average);
            double ratingAjustado1 = ratingItem1 - users[i].average;
            double ratingAjustado2 = ratingItem2 - users[i].average;

            sumaNumerador += ratingAjustado1 * ratingAjustado2;
            sumaDenominador1 += ratingAjustado1 * ratingAjustado1;
            sumaDenominador2 += ratingAjustado2 * ratingAjustado2;
        }
    }

    if (sumaDenominador1 == 0 || sumaDenominador2 == 0) {
        return 0; // Retornar 0 si no hay datos suficientes
    }

    return sumaNumerador / (sqrt(sumaDenominador1) * sqrt(sumaDenominador2));
}

double buscarCosenoAjustado(int item1, int item2, int totalElementos) {
    for (int i = 0; i < totalElementos; i++) {
        // Verifica si los ids coinciden sin importar el orden
        if ((matrizC[i].itemId1 == item1 && matrizC[i].itemId2 == item2) ||
            (matrizC[i].itemId1 == item2 && matrizC[i].itemId2 == item1)) {
            return matrizC[i].CosenoAjustado;
        }
    }
    // Si no se encuentra, devolver un valor que indique la ausencia
    return -1.0;  // Por ejemplo, puedes devolver -1 si no encuentras el valor
}

double hallarMatrizCosenoAjustado(int userPredict, int itemPredict) {

    // Falta ver, validarsi items ya fue califificado, o si el item no existe...

    // Validar si el ususario selecionado esta en la lista
    // Modificar para que segun el algoritmo elija una estructura diferente...
    for (int i = 1; i <= usersNums; i++) {
        //users[i].average = calcularPromedioUsuario(i);
        printf("User: %d, promedio = %lf\n", i, users[i].average);
    }

    /*int index = 0;
    int totalCombinaciones = maxItems * (maxItems - 1) / 2;

    printf("Total combinaciones: %d\n", totalCombinaciones);
    printf("Max Items: %d\n", maxItems);

    matrizC = (MatrizCoseno*)malloc(totalCombinaciones * sizeof(MatrizCoseno));

    if (matrizC == NULL) {
        fprintf(stderr, "Error: no se pudo asignar memoria para matrizC\n");
        exit(EXIT_FAILURE);
    }


    for (int i = 0; i < maxItems; i++) {
        for (int j = i + 1; j < maxItems; j++) {
            

            matrizC[index].itemId1 = movies[i].movieId;
            matrizC[index].itemId2 = movies[j].movieId;
            matrizC[index].CosenoAjustado = hallarSimilitudDeCosenoAjustado(movies[i].movieId, movies[j].movieId);
            printf("%d)Similitud entre Item %s y Item %s: %.4lf\n", index+1, movies[i].name, movies[j].name, matrizC[index].CosenoAjustado);
            index++;
        }
    }*/



    // Iniciamos con la predicción(ejemplo: DAVID):

    double userRatingsModified[users[userPredict].itemCount];
    double minRating = __DBL_MAX__, maxRating = __DBL_MIN__;

    for (int i = 0; i < users[userPredict].itemCount; i++) {
        double currentRating = users[userPredict].item[i].rating;

        if (currentRating < minRating) {
            minRating = currentRating;
        }

        if (currentRating > maxRating) {
            maxRating = currentRating;
        }

    }

    // Al final del bucle, ya tendrás los valores de minRating y maxRating
    //printf("Min Rating: %lf, Max Rating: %lf\n", minRating, maxRating);
    double sumaMatriz = 0.0;
    double numeradorPredict = 0.0;
    double maximin = maxRating - minRating;

    for (int i = 0; i < users[userPredict].itemCount; i++) {
        double currentRating = users[userPredict].item[i].rating;
        double currentCoseno = hallarSimilitudDeCosenoAjustado(itemPredict, users[userPredict].item[i].itemId);
        /*if(currentCoseno == 0) {
            continue;            
        }*/
        //double currentCoseno = buscarCosenoAjustado(itemPredict, users[userPredict].item[i].itemId, totalCombinaciones);
        printf("Matriz entre el elegido y los calificados: %lf\n", currentCoseno);

        userRatingsModified[i] = ((2 * (currentRating - minRating)) - (maximin)) / (maximin);
        printf("RatingNormalizado: %lf\n", userRatingsModified[i]);
        numeradorPredict += (userRatingsModified[i] * currentCoseno);
        sumaMatriz += fabs(currentCoseno);
    }

    printf("numeradorPredict: %lf\n", numeradorPredict);

    printf("Suma(Denominador): %lf\n", sumaMatriz);

    double predictNomalized = numeradorPredict / sumaMatriz;

    printf("Rating Predecido Norm: %lf\n", predictNomalized);

    double predictDesNomalized = 0.5*((predictNomalized + 1) * (maxRating - minRating)) + minRating;

    printf("Rating Predecido DesNorm: %lf\n", predictDesNomalized);


    free(matrizC);

    return predictDesNomalized;

}

double hallarPrediccion(int usuarioIndex, int itemIndex ) {

    if (users[usuarioIndex].itemCount == 0 || usuarioIndex > usersNums || usuarioIndex < 0) {
        printf("\nEl usuario no existe\n");
        exit(1);
    } else if (encontrarRatingSegunId(usuarioIndex, itemIndex) != -1.0){
        printf("\nEl usuario ya califico al item: %d\n", itemIndex);
        exit(1);
    }

    if (readItems("../dataset/movies.csv", ",", 1, itemIndex) == 0) {
        printf("\nNo existe este item: %d\n", itemIndex);
        exit(1);
    }

    double prediccion = 0.0;

    printf("Similitud de Coseno Ajustado:\n");
    prediccion = hallarMatrizCosenoAjustado(usuarioIndex, itemIndex);


    printf("\nSe predice que: El usuario %d, calificara a el item: %s, con: %lf\n", usuarioIndex, movies[itemIndex-1].name, prediccion);

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
    readData("../dataset/ratings.csv", ",", 1, 0);
    //readData("../dataset/ratingTest.csv", ",", 1, 0);
    printf("Numero de usuarios: %d\n", usersNums);
    printf("Numero max de items: %d\n", maxItems);

    //imprimirUsuarios();

    int userPredict = 606;
    int itemPredict = 10;

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    //hallarMatrizCosenoAjustado(userPredict, itemPredict);
    hallarPrediccion(userPredict, itemPredict);
    //printf("resultado: %lf\n", hallarSimilitudDeCosenoAjustado(52245,95858));
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