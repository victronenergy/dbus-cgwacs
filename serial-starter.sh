#! /bin/bash
exec 2>&1

#verbose=
#logfile=

log(){
  test -z "$myname" && myname=`basename $0`
  #logger -t "$myname" -p "user.info" "$*"
  test -t 1 && echo "`date +%Z-%Y.%m.%d-%H:%M:%S` $*"
  test -z "$logfile" || echo "`date +%Y.%m.%d-%H:%M:%S` [$$] $*" >> $logfile
}

SERIAL_TTYS="/dev/ttyO2"

# only add ttyO0 when not used as console
if [ -f /service/vegetty/down ]; then
	SERIAL_TTYS="${SERIAL_TTYS} /dev/ttyO0"
fi

declare -A VEDIRECT
declare -A GPS
declare -A CGWACS

BASE_DIR='/opt/color-control'
VEDIRECT='vedirect-interface'
GPS='gps-dbus'
CGWACS='dbus-cgwacs'
PROGRAMS=( VEDIRECT GPS CGWACS )

SERVICE_DIR="/var/services"

GPS_LOCK="/var/lock/gps.lock"

rm -rf "/service/$PROGRAM1.*"
rm -rf "/service/$PROGRAM2.*"
rm -rf /var/service/*
rm -rf /var/lock/tty*.lock

function start_service() {
    # lock tty with program name
    #log "INFO: Lock $TTY for $PROGRAM"
    echo "$PROGRAM" > "$DEV_LOCK"
    # start service once
    log "INFO: Start service $SERVICE once"
    svc -u "/service/$SERVICE/log"
    svc -o "/service/$SERVICE"
}

echo "serstart starting"
while true; do
    USB_TTYS=`ls /dev/ttyUSB? 2> /dev/null`
    TTYS=$SERIAL_TTYS" "$USB_TTYS
    for TTY_DEV in $TTYS;
    do
        TTY=${TTY_DEV#/*/} # get part after forward slash
        DEV_LOCK="/var/lock/$TTY.lock"
        if [ ! -f  "$DEV_LOCK" ]; then
            INDEX_FILE="/tmp/$TTY.index"
            # Check program index file, init if not exists
            if [ ! -f "$INDEX_FILE" ]; then
                echo "0" > "$INDEX_FILE"
            fi
            INDEX=`cat $INDEX_FILE`
            ITEM="${PROGRAMS[$INDEX]}"
            PROGRAM=${!ITEM}
            #log "INFO: service:$PROGRAM.$TTY index:$INDEX"
            if [ -d "$BASE_DIR/$PROGRAM/service" ]; then # if program has service
                SERVICE="$PROGRAM.$TTY"
                #log "INFO: Using service $SERVICE"
                if [ ! -d "/service/$SERVICE" ]; then # if service not already exists
                    log "INFO: Create daemontools service for tty $TTY and service name $SERVICE"
                    # copy service
                    cp -a "$BASE_DIR/$PROGRAM/service/." "$SERVICE_DIR/$SERVICE"
                    # Replace / to \/ for sed
                    TTY_DEV=${TTY_DEV//\//\\\/}
                    # Patch run files for tty device and service name
                    sed -i "s/TTY_DEV/$TTY_DEV/" "$SERVICE_DIR/$SERVICE/run"
                    sed -i "s/TTY.lock/$TTY.lock/" "$SERVICE_DIR/$SERVICE/run"
                    sed -i "s/.TTY/.$TTY/" "$SERVICE_DIR/$SERVICE/log/run"
                    # Create symlink to /service
                    ln -sf "$SERVICE_DIR/$SERVICE" "/service/$SERVICE"
                    # wait for svscan to find service
                    sleep 6
                fi

                if [ "$PROGRAM" == "$GPS" ]; then # if GPS
                    if [ ! -f  "$GPS_LOCK" ]; then # if no lock
                        log "INFO: Single $GPS lock on $TTY"
                        echo "$TTY" > "$GPS_LOCK"
                        start_service
                    fi
                else
                    start_service
                fi
            else
                log "ERROR: No service directory found in $BASE_DIR/$PROGRAM)"
            fi
            (( INDEX++ ))
            if [ "$INDEX" -ge "${#PROGRAMS[@]}" ]; then
                echo "0" > "$INDEX_FILE"
            else
                echo "$INDEX" > "$INDEX_FILE"
            fi
        fi
    done
    sleep 2
done
