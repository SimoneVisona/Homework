#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "weightedAverage.h"

void double_zeroElements(double array[], int N){
    for(int i=0; i<N; i++)
        array[i] = 0;
}

void int_zeroElements(int array[], int N){
    for(int i=0; i<N; i++)
        array[i] = 0;
}

bool appears(double array[], double value, int N){
    for (int i=0; i<N; i++){
        if (array[i] == value)
            return true;
    }
    return false;
}

void create_singolarArray(double original_array[], double array_modified[], int N){
    int index = 0;
    for (int i=0; i<N; i++){
        for (int j=0; j<N; j++){
            if (!(appears(array_modified, original_array[i], N))){
                array_modified[index] = original_array[i];
                index++;
            }
        }
    }
}

void calculate_eachWeight(double original_array[], double array_modified[], int array_pesi[], int N){
    int count;
    for(int i=0; i<N; i++){
        count = 0;
        if((int)array_modified[i] != 0){
            for(int j=0; j<N; j++){
                if (array_modified[i] == original_array[j])
                    count++;
            }
            array_pesi[i] = count;
        } else {
            i = N;
        }

    }
}

double calculate_weighted_average(double array_singoli_elementi[], int array_pesi[], int N){
    double sum = 0;
    int peso_tot = 0;
    double res;

    for (int i=0; i<N; i++){
        if (array_singoli_elementi[i] != 0){
            sum = sum + (array_singoli_elementi[i]*array_pesi[i]);
        }
        if ((int) array_singoli_elementi[i] == 0)
            i = N;
    }

    for (int i=0; i<N; i++){
        if (array_pesi[i] != 0){
            peso_tot = peso_tot + array_pesi[i];
        }
        if (array_pesi[i] == 0)
            i = N;
    }
    res = sum/peso_tot;
    return res;
}


double weighted_average(double array_of_values[], int LengthArray){

/*A causa della lunghezza variabile dei vettori, sono obbligato ad inizializzarli con ogni singolo elemento pari a zero.
Se non facessi ciò ad un certo punto del calcolo un elemento del vettore assume valori casuali, sballando tutto*/

double array_with_single_elements[LengthArray];
double_zeroElements(array_with_single_elements, LengthArray);
int array_weights[LengthArray];
int_zeroElements(array_weights, LengthArray);
double res;

create_singolarArray(array_of_values, array_with_single_elements, LengthArray);

/*printf("New array: \n");

for (int i=0; i<LengthArray; i++){
    printf("%f \n", array_with_single_elements[i]);
}*/

/*così facendo avrò una parte dell'array piena e l'altra composta da tutti zeri. Infatti la lunghezza N dell'array 2 serve nel caso peggiore
ovvero quando array contiene tutti elementi diversi uno dall'altro. Si può migliorare tale algoritmo utilizzand perciò le liste concatenate al fine
di massimizzare il rendimento del programma dal punto di vista dell'occupazione di memoria ed inoltre questo permette di eliminare il vincolo sulla
conoscenza del numero di dati in ingresso (informazione fondamentale per la costruzione dell'array nella sua lunghezza)*/

calculate_eachWeight(array_of_values, array_with_single_elements, array_weights, LengthArray);

/*printf("Weight of elements: \n");

for (int i=0; i<LengthArray; i++){
    printf("%i \n", array_weights[i]);
}*/

res = calculate_weighted_average(array_with_single_elements, array_weights, LengthArray);

return res;

}

double average(double buffer[], int N){
double res;
int count = 0;
double sum = 0;

for (int i=0; i<N; i++){
    if(buffer[i] > 0){
        sum = sum + buffer[i];
        count++;
    }
}

res = sum/count;
return res;
}
