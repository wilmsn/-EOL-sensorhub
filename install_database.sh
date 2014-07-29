#!/bin/sh

if [ -e /var/database/sensorhub.db ]; then
  echo "Datenbank /var/database/sensorhub.db vorhanden, nicht neu installiert"
  echo "Um neu zu installieren bitte die Datei /var/database/sensorhub.db löschen"
  echo "Achtung: dabei gehen alle gespeicherten Daten verloren!!!!!!"
else
  echo "Installiere Database nach /var/database"
  if [ -d /var/database ]; then
    cp ./sensorhub.db /var/database
    chmod 666 /var/database/sensorhub.db
  else
    echo "Bitte das Verzeichnis /var/database erstellen"
  fi
fi

