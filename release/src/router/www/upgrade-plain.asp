<html><head><title>Aktualizacja Firmware</title></head>
<body>
<h1>Aktualizacja Firmware</h1>
<b>UWAGA, B. WAŻNE, DOKŁADNIE PRZECZYTAĆ!</b>
<ul>
<li><h2>Na tej stronie nie jest wyświetlana informacja o statusie postępu wgrywania nowego oprogramowania
i jej zawartość nie ulega zmianom po wciśnięciu przycisku "Aktualizuj". Po poprawnym zakończeniu procesu aktualizacji
zostanie wyświetlona nowa strona i dopiero to oznacza zakończenie aktualizacji.
<li>Proces aktualizacji firmware może trwać do 3 minut. Pod żadnym pozorem nie wolno go przerywać w żaden sposób! Nie próbuj odświeżać
tej strony ani dokonywać jakichkolwiek operacji na routerze aż do zakończenia procesu aktualizacji. Każda taka próba może się skończyć uszkodzeniem routera!
Przed rozpoczęciem aktualizacji upewnij się, że plik firmware został dobrany odpowiednio dla Twojego urządzenia! Pomyłka spowoduje uszkodzenie Twojego sprzętu.
</ul></h2>

<br>
<form name="firmware_upgrade" method="post" action="upgrade.cgi?<% nv(http_id) %>" encType="multipart/form-data">
<input type="hidden" name="submit_button" value="Aktualizacja">
Firmware: <input type="file" name="file"> <input type="submit" value="Aktualizuj">
</form>
</body></html>
