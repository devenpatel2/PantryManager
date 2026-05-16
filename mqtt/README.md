# MQTT broker

Mosquitto in a container, listening on host port 1883 with anonymous access enabled. Used by the PantryServer and PantryClient — see the top-level [README](../README.md) for the bigger picture.

## Start / stop / logs

```bash
cd /home/dpa/PantryManager/mqtt

docker compose up -d        # start (detached)
docker compose down         # stop
docker logs -f mqtt_broker  # tail logs
```

## Config

`config/mosquitto.conf` is mounted into the container. Two lines:

```
listener 1883
allow_anonymous true
```

No auth — this lives on a single home LAN. If the broker is ever exposed beyond the LAN, add credentials (`password_file` + `mosquitto_passwd`) and `allow_anonymous false`.

## Replacing the old bash alias

This replaces the previous Pi-local `mqtt-broker` bash alias and the ad-hoc `$HOME/mqtt_config/mosquitto.conf`. Once you confirm the compose setup is working:

```bash
# Stop the old container if it's still running under a different name
docker stop mqtt_broker 2>/dev/null
docker rm mqtt_broker 2>/dev/null

cd /home/dpa/PantryManager/mqtt
docker compose up -d
```

Then remove the `mqtt-broker` function from `~/.bash_aliases` on the Pi (optional cleanup).
