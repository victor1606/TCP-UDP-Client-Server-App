Calugaritoiu Ion-Victor 322CA

Protocoale de Comunicatii - Tema 2 

Aplicatie client-server TCP si UDP pentru gestionarea mesajelor

- subscriber.cpp : am extras adresa server-ului, port-ul si id-ul, am deschis
socket-ul de tcp si m-am conectat la server; dupa realizarea conexiunii,
am trimis id-ul catre server si am setat file descriptorii socketilor; dupa
primirea unui mesaj, este verificat tipul descriptorului si in cazul in care
este de tip STDIN, mesajul primit este parsat si se verifica daca s-a primit o
comanda de tip "exit" (se inchide socket-ul), "subscribe" sau "unsubscribe"
(se trimit parametrii respectivi si se printeaza mesajele corespunzatoare).
in cazul in care se primeste un mesaj de la server, se verifica daca mesajul
a fost primit in totalitate; in caz contrar, se trimit cereri pana cand 
mesajul este trimis complet, si se afiseaza la STDOUT mesajul corespunzator;

- server.cpp : am deschis socketii de udp si tcp, am completat adresele si
portul, am apelat "bind" pt ambii socketi si am setat file descriptorii; in 
cadrul buclei while, am verificat ce fel de mesaj este primit: 

    - UDP : am completat structura "message" cu adresa si portul primit si am
    verificat daca valoarea propriu-zisa a mesajului este de tip INT, 
    SHORT_REAL, FLOAT sau STRING; pentru fiecare tip, se apeleaza functia 
    auxiliara descrisa in fisierul "udp_types.cpp", convertind datele 
    corespunzator; considerand topic-ul primit, parcurg map-ul 
    "subscr_by_topic" de tip <string, vector<subscribers> > si memorez vectorul
    de subscriberi aferent topic-ului; vectorul este parcurs iar mesajul dorit
    este trimis subscriberilor;

    - TCP : in cazul in care se conecteaza un client, parcurg vectorul de
    clienti conectati si verifica daca este un client nou; daca clientul este
    deja conectat, se inchide socket-ul si se printeaza mesajul corespunzator;
    daca este un nou client, este adaugat in vectorul curr_clients, fiind 
    actualizate map-urile map_socket_id si map_id_socket + se printeaza mesajul
    corespunzator unei noi conexiuni cu un client; in cazul receptionarii unei
    cereri de la un client existent, se verifica daca mesajul primit este 
    "subscribe", "unsubscribe" sau daca clientul se deconecteaza (se sterge 
    din vectorul de curr_clients, se actualizeza map-urile, se printeaza un
    mesaj corespunzator); daca comanda este de tip "subscribe", se actualizeaza
    structura de tip subscription cu id-ul si sf-ul primit; se parcurge lista
    de topic-uri si se verifica daca acelasi client s-a abonat la acelasi
    topic dar cu un sf diferit, caz in care acesta este actualizat; daca se
    aboneaza la alt topic, este adaugat in map-ul subscr_by_topic la vectorul
    corespunzator; pentru comenzile de tip "unsubscribe", se actualizeaza toate
    structurile, fiind eliminat id-ul client-ului din lista respectiva;

    - STDIN : in cazul in care comanda primita este "exit", sunt inchisi toti
    socketii clientilor + socketul de TCP si UDP;

Definirea macro-urilor si a structurilor folosite este facuta in fisierul
structures.h;