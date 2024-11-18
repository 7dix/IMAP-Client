# IMAP klient

## Autor

- **Jméno:** Marek Tenora
- **Login:** xtenor02
- **Datum vytvoření:** 18.11.2024

## Popis programu
 Jednoduchý IMAP klient napsaný v C++, který umožňuje připojení k IMAP serveru, autentizaci uživatele a stahování e-mailových zpráv z vybrané poštovní schránky. Program podporuje zabezpečené připojení pomocí TLS a umožňuje stahovat pouze nové zprávy nebo pouze hlavičky zpráv.

### Rozšíření a omezení

- **Rozšíření:**
  - Adresářová struktura uložených emailů.
  - Zotavení z chyby serveru kdy nepošle vyžádanou zprávu.

- **Omezení:**
  - Nepodporuje odesílání e-mailů.
  - Podporuje pouze základní autentizaci.

## Příklad spuštění

```bash
./imapcl -s imap.example.com -p 993 -T -a auth.txt -b INBOX -o ./mails
```
## Argumenty
### Povinné argumenty
- adresa IMAP serveru, ke kterému se připojujete,
- -a: cesta k autentizačnímu souboru, který obsahuje přihlašovací údaje,
- -o: cesta k adresáři, kam se budou ukládat stažené e-maily.
### Volitelné argumenty
- -p : specifikace portu. Výchozí hodnoty jsou 143 pro nešifrované připojení a 993 pro TLS,
- -T: použití protokolu TLS pro zabezpečení připojení,
Následující parametry se používají pouze s volbou -T:
- - -c: cesta k souboru s certifikátem,
- - -C: cesta k adresáři s certifikáty (výchozí hodnota je /etc/ssl/certs),
- -n: stáhne pouze nové zprávy,
- -h: stáhne pouze hlavičky zpráv,
- -b: název poštovní schránky, kterou chcete použít (výchozí je INBOX).

## Seznam odevzdaných souborů
```
/src
    main.cpp
    ArgumentsParser.cpp
    ArgumentsParser.h
    AuthReader.cpp
    AuthReader.h
    ImapClient.cpp
    ImapClient.h
    ImapParser.cpp
    ImapParser.h
    FileHandler.cpp
    FileHandler.h
/tests
    ArgumentsParser_test.cpp
    AuthReader_test.cpp
    ImapClient_test.cpp
    ImapParser_test.cpp
    main_test.cpp
/docs
    uml.md
    sequence.md
    sequenceMessageFailed.md
Makefile
test_Makefile
LICENSE
README.md
manual.pdf
```