# FieldManager (greek only)

Υποτυπώδης in-memory βάση δεδομένων για αγρότες.

Οι χρήστες μπορούν να δημιουργήσουν καταχωρήσεις για τις φυτείες και τα χωράφια τους. Για κάθε καταχώρηση αποθηκεύονται διάφορες πληροφορίες, όπως το είδος της φυτείας, τιμή λιπάσματος,  στρέμματα χωραφιού κτλ.

Με βάση αυτές τις πληροφορίες υπολογίζονται τα πιθανά έσοδα/έξοδα/κέρδη. Όσο πιο πολλές πληροφορίες δοθούν από τον χρήστη για μία φυτεία, τόσο πιο έγκυρα θα είναι τα αποτελέσματα.

Παράλληλα υποστηρίζεται η δημιουργία ραβδογραμμάτων για την εύκολη σύγκριση εσόδων/εξόδων/κερδών μεταξύ των διάφορων φυτειών.

![Gif1](https://i.imgur.com/InEHPtH.gif)

Τέλος, το πρόγραμμα παρέχει widget για ενημέρωση καιρού, καθώς και widget για αγροτικά νέα.

![Gif2](https://i.imgur.com/7yIKSXs.gif)

## Dependencies


* [json-parser](https://github.com/udp/json-parser)

* [libmrss](https://github.com/bakulf/libmrss)

* [libnxml](https://github.com/bakulf/libnxml)

* [libmctrl](http://www.mctrl.org)

Στα Releases υπάρχει [έτοιμο build για 64bit Windows](https://github.com/Gikoskos/FieldManager/releases/download/1.0.0/FieldManager_x64.zip).

Για να κάνω build χρησιμοποιώ MSYS2 + MinGW-w64. Τα libmrss  και libnxml παρέχουν μόνο autotools build systems και γι' αυτό χρειάζεται το MSYS2. Το json-parser είναι μόνο δύο αρχεία που τα βάζω στο root του repo.

## License

[LICENSE.txt](#LICENSE.txt)