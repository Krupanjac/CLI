# Vodič kroz izvršavanje CLI-a

## 1. Pokretanje programa
- Ulazna tačka je `main()` u fajlu `main.cpp`, koji jednostavno konstruše objekat `Interface`. Konstruktor (`Interface.cpp`) postavlja `parser`/`command` na `nullptr` i odmah poziva `Interface::run()`, tako da se kontrola ne vraća u `main()`.

## 2. Interaktivna petlja (`Interface::run`)
- Znak prompta (`PSIGN`, podrazumevano `$`) čuva se lokalno i koristi dok se ne promeni komandom `prompt`.
- Svaki ciklus alocira `PromptCommand` (`Command.cpp`), poziva `execute()` da odštampa `"$ "`, briše komandu, a zatim kreira novi `Parser`.
- Konstrukcija parsera (`Parser.cpp`) izvršava `std::cin >> stream`, što poziva `Stream::operator>>` da parsira jednu celu liniju, uključujući pipeline-ove.
- Parsirani podaci i referenca na prompt prosleđuju se u `PipelineExecutor::run()` (`PipelineExecutor.cpp`). Kada se izvršavanje završi, parser se oslobađa i petlja počinje iznova.

## 3. Čitanje i tokenizacija ulaza (porodica `Stream`)
- `Stream::operator>>` (`Stream.cpp`) čita celu liniju, proverava ograničenje `MAX_SIZE` (512 karaktera) i ispisuje grešku 10 (prazan unos) ili 11 (predugačko) pre prekida.
- `Stream::split` deli liniju po `|`, sklanjajući razmake kako bi se dobile faze pipeline-a.
- Za svaku fazu `parseRedirections` izdvaja završne tokene preusmeravanja `< fajl`, `> fajl` ili `>> fajl` (razmaci nisu obavezni) i čuva ih; preostali tekst se prosleđuje u `InputStream`.
- `InputStream::parse` prolazi kroz tokene, poštujući navodnike. Opcije (`-x`) se automatski detektuju; hvataju se do tri argumenta i `explicitArgument` se postavlja kada je prvi argument eksplicitno zadat.
- Ako segment pipeline-a sadrži samo beline, odbacuje se.

## 4. Automatsko popunjavanje argumenata
- Komande koje mogu da koriste fajl/stdin kao ulaz (`echo`, `wc`, `head`, `batch`) odlažu popunjavanje argumenta do posle parsiranja:
  - Ako je navedeno `< fajl` i nije dat eksplicitni argument, sadržaj fajla se učitava u prvi argument (u navodnicima).
  - Ako nema argumenta i nema `< fajl`, prvi segment pipeline-a može da traži višelinijski unos do `EOF`/prazne linije, opet u navodnicima.
- `.txt` argumenti za `echo`, `wc`, `head`, `batch` i `tr` se detektuju pomoću `isTxtFileCandidate` i zamenjuju se `FileStream` objektom koji učitava sadržaj fajla i stavlja ga u navodnike.
- `tr` podržava delimične ulaze: ako su zadati samo tokeni zamene, tekst se dobavlja putem preusmeravanja ulaza ili interaktivnog unosa (samo za prvi segment pipeline-a).

## 5. Izvršavanje pipeline-a (`PipelineExecutor::run`)
- `PipelineExecutor` prolazi kroz parsirane čvorove koristeći `Parser::processAll()` (koji prolazi `Stream` jednom).
- Validacija preusmeravanja se vrši pre kreiranja komande: mešanje `<` sa eksplicitnim argumentima za `echo`/`wc`/`head`/`batch` okida `CommandFactory::handleCommand(SYNTAX_ERROR, ...)`.
- `CommandFactory::createCommand` validira opcije/argumente i vraća konkretnu `Command` instancu ili `nullptr` u slučaju greške. Komanda `prompt` namerno ažurira znak prompta i vraća `nullptr`.
- Rukovanje izlazom:
  - Ako se trenutni segment povezuje cevovodom sa sledećim i nema `>`, `std::cout` se privremeno preusmerava na `std::ostringstream` radi hvatanja izlaza.
  - Sa `>`/`>>`, `std::cout` se preusmerava na fajl tok (otvoren u režimu dodavanja ili prekucavanja). Nakon izvršenja tok se zatvara — a ako cilj ima ekstenziju `.txt`, njegov sadržaj se ispisuje nazad na konzolu.
- Uhvaćen izlaz iz pipeline-a se stavlja u navodnike i ubrizgava u sledeći `InputStream`:
  - `tr` zadržava svoje opcione drugi/treći argument pri prevezivanju.
  - `echo`, `wc`, `head` i `batch` koriste citirani tekst kao svoj primarni argument.

## 6. Katalog komandi (`Command.cpp`)
- **PromptCommand**: ispisuje trenutni prompt.
- **EchoCommand**: zahteva citirani argument (ili `.txt` fajl čiji sadržaj parser zamenjuje citiranim tekstom); ispisuje raznavodničeni tekst. Necitirani tokeni se odbijaju prilikom validacije.
- **TimeCommand**/**DateCommand**: ispis trenutnog vremena (`std::put_time`) i datuma kompilacije (`__DATE__`).
- **ClearCommand**: poziva `system("cls")`.
- **ExitCommand**: `exit(0)`.
- **TouchCommand**: kreira novi `.txt` fajl (argument ne sme biti u navodnicima i mora se završavati na `.txt`); baca grešku 7 ako već postoji ili 8 ako kreiranje ne uspe.
- **WcCommand**: broji reči (`-w`) ili karaktere (`-c`, podrazumevano ako se izostavi). Prihvataju se samo `-w` ili `-c`; navodnici se skidaju pre brojanja.
- **HelpCommand**: ispisuje pomoć/korisničko uputstvo.
- **TruncateCommand**: skraćuje postojeći `.txt` fajl korišćenjem `std::ofstream(..., std::ios::trunc)`; fajl mora već postojati.
- **RmCommand**: briše postojeći `.txt` fajl pomoću `std::remove`, prijavljuje uspeh ili neuspeh.
- **TrCommand**: zamenjuje sva pojavljivanja citiranog `what` nizom `with` u citiranom `input` tekstu.
- **HeadCommand**: ispisuje prvih `n` linija citiranog ulaza; opcija mora biti `-n` praćena najviše 5 cifara (npr. `-n10`).
- **BatchCommand**: deli citirani skript na pojedinačne linije, svaku ponovo parsira u `Stream` i izvršava sa istim pravilima validacije/pipeline-a (uključujući ugnježdene pipeline-ove i preusmeravanja).

## 7. Validacija i prijava grešaka (`CommandFactory`)
- Svaka komanda ima svoju `validate*` rutinu koja sprovodi pravila za opcije, navodnike i fajl provere (`validateFileForOpen`).
- Greške prijavljuje `handleCommand`, koji štampa problematični fragment komande, podvlači ga karakterom `~` i dodaje kontekstualnu poruku o grešci (`Error code 1`–`4`).
- `createCommand` se prebacuje na `UNKNOWN_COMMAND` kada ne postoji odgovarajući obrađivač.

## 8. Pregled kodova grešaka (poreklo)
- 1 — Neispravan argument (greška validacije)
- 2 — Neispravna opcija (greška validacije)
- 3 — Sintaksna greška (npr. kombinovanje `<` sa eksplicitnim argumentima za komande koje same obezbeđuju ulaz)
- 4 — Nepoznata komanda (nema handler-a u fabrici)
- 5 — Ne može da se otvori fajl: <putanja> (neuspešno otvaranje ulaznog fajla u parseru)
- 6 — Greška pri čitanju fajla … (`FileStream` neuspeh)
- 7 — Fajl već postoji (iz `TouchCommand`)
- 8 — Ne može da se kreira/otvori fajl (iz kreiranja `TouchCommand` ili neuspeh otvaranja izlaza)
- 10 — Prazan unos (iz `Stream::operator>>`)
- 11 — Unos premašuje MAX_SIZE (iz `Stream::operator>>`)
  
Napomene:
- Neuspeh `rm` komande ispisuje tekstualnu poruku (`Error deleting file: …`) umesto numeričkog koda.
- U interaktivnom režimu, upis u `.txt` preko `>`/`>>` echo-vuje sadržaj fajla nazad na stdout; unutar `batch` to echo važi samo za dodavanje (`>>`).

## 9. Napomene i ograničenja
- Deljenje pipeline-a je doslovno po `|` i ne obraća pažnju na navodnike; `|` unutar navodnika i dalje deli pipeline.
- Preusmeravanja se parsiraju samo na kraju segmenta; važi samo poslednje pojavljivanje datog preusmeravanja. Preusmeravanje stderr-a nije podržano.
- Interaktivno čitanje sa stdin (za `echo`, `wc`, `head`, `batch` i delimični `tr`) dešava se samo za prvi segment pipeline-a i zaustavlja se na praznoj liniji ili liniji `EOF`.
- Cevovod prosleđuje uhvaćen stdout kao jedan citirani argument sledećoj komandi. Samo `tr`, `echo`, `wc`, `head` i `batch` troše ovaj ubrizgani argument.
- Brojanje karaktera u `wc -c` uključuje znakove novog reda prisutne u ulazu.
- Komande vezane za fajlove primenjuju `.txt` ekstenzije; `truncate`/`rm` takođe zahtevaju da fajl postoji.

## 10. Detaljno o batch komandi
- `BatchCommand::execute` (`Command.cpp`) prolazi linije iz svog citiranog skripta.
- Svaka linija se parsira novim `Stream`-om i izvršava sekvencijalno, koristeći isto hvatanje pipeline izlaza i logiku preusmeravanja kao u interaktivnom režimu.
- Preusmeravanje izlaza unutar batch komandi poštuje režime dodavanja/prepisivanja i ispisuje `.txt` fajlove nazad na konzolu kada se dodaju.

## 11. Prilagođavanje prompta (komanda `prompt`)
- Validacija prihvata jedan karakter, ili neokružen navodnicima ili u navodnicima (`"$"`). Više-karakterni unosi izazivaju `INVALID_ARGUMENT`.
- Pri uspehu, prompt sačuvan u `Interface::run` se ažurira za naredne iteracije; ne kreira se i ne izvršava poseban objekat komande.

## 12. Rekapitulacija toka kontrole
1. `main()` konstruše `Interface`; `Interface::run` ulazi u REPL petlju.
2. Prompt se prikazuje preko `PromptCommand`.
3. `Parser` čita ulaz, `Stream` deli pipeline, tokenizuje i rešava preusmeravanja/učitavanja iz fajla.
4. `PipelineExecutor` validira svaku fazu, instancira komande, upravlja preusmeravanjem/cevodom i izvršava ih.
5. Izlazi i greške se prikazuju odmah; prompt se ponovo pojavljuje dok se ne pozove `exit`.

Ovaj vodič pokriva sve učesnike iz `.cpp`/`.h` fajlova kako bi CLI mogao da se predstavi korak po korak na projektnom prikazu.
