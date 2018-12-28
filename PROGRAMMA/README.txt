La soluzione presenta due cartelle: una dedicata al singolo file CLIENT che simula la persona in remoto che "pilota" il dispositivo e una contenente al programma intero. 
Per quanto riguarda la compilazione del programma, dal prompt eseguire il codice:
"gcc -Wall -o NOMEESEGUIBILE main.c parsingInput.c weightedAverage.c threads.c monitor.c server.c draw.c -lpthread"
e quindi lanciare il programma con una descrizione del tipo "./NOMEESEGUIBILE A B C D" dove:

-A = posizione muro negativo;

-B = posizione muro positivo;

-C = tempo ciclo del viewer;

-D = tempo cilo del controller;

Una volta che il programma è in esecuzione si può compilare su prompt dedicato il client cone la seguente linea di codice: 
"gcc -Wall -o NOMEESEGUIBILE client.c -lpthread"e quindi lanciare il programma con una descrizione del tipo "./NOMEESEGUIBILE IP" dove IP è l'indirizzo IP della macchina (es:192.168.56.1).




UTILIZZO CHIUSURA PROCESSI:

Il programma è stato strutturato in modo tale per cui, una volta decisa la chiusura del programma complessivo (contenente il server e le varie thread interface model viewer controller) per eseguirla
bisogna mandare un SIGINT da prompt. 
A questo punto, come verrà visualizzato a schermo, al CLIENT è chiesto l'invio di un ultimo dato arbitrario per concludere in modo corretto la connessione.

Viceversa se la chiusura è invece dettata dal CLIENT allora questo ultimo per imporre la disconnessione (come viene descritto da schermo) dovrà mandare un codice "speciale" definito come "0.01".
Se il client invia il valore "0.01" allora questo è portato a morire e il server resta in attesa di una nuova connessione.