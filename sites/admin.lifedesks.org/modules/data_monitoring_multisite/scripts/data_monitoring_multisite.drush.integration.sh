#!/usr/local/bin/drush
#!/bin/bash
#!/bin/sh

# Bash script to run data monitoring procedures across all sites.
# For integration testing.

DRUPAL_PATH='/data/www/lifedesk'
ADMIN_URL='http://admin.intlifedesks.org'
LOG_FILE_PATH="/data/backups/data_monitoring/log_$( date +%Y%m%d_%H%M%S ).txt"
EMAIL='lddev@eol.org'
SUBJECT='Data Monitoring Execution Error'

SITES=$( drush -r ${DRUPAL_PATH} -l ${ADMIN_URL} php-eval "_data_monitoring_multisite_drush_get_sites()" )

if [[ $SITES == a* ]] ; then

  for SITE_SHORTNAME in $SITES ; do
    drush -r ${DRUPAL_PATH} -l http://${SITE_SHORTNAME}.intlifedesks.org php-eval "data_monitoring_run()" >> $LOG_FILE_PATH
    wait $!
    chown -R apache:apache ${DRUPAL_PATH}/files/${SITE_SHORTNAME}/data-monitoring
  done

  drush -r ${DRUPAL_PATH} -l ${ADMIN_URL} php-eval "data_monitoring_multisite_run()" >> $LOG_FILE_PATH

else

  echo "$(date -u) DATA MONITORING LOG
          ERROR: Data monitoring did not execute on any site.
          MESSAGE: Unable to get site shortnames, _data_monitoring_multisite_drush_get_sites did not return an array." >> $LOG_FILE_PATH

fi
ERR_COUNT=$( grep -c -i "ERROR" "$LOG_FILE_PATH" )
if [ -z $ERR_COUNT ] ; then
  echo "Data monitoring error count is null there was a problem executing grep on $LOG_FILE_PATH" | mail -s "$SUBJECT" $EMAIL
elif [ $ERR_COUNT -gt 0 ] ; then
  echo "Found $ERR_COUNT error(s) in data monitoring execution. Check log file $LOG_FILE_PATH" | mail -s "$SUBJECT" $EMAIL
fi