<?php

// $Id: yubikey.inc,v 1.1.2.1 2009/02/04 06:23:35 rubinj Exp $

/*
 * @file
 * YubiKey utility functions.
 */

function yubikey_keyid($otp) {
  return drupal_substr($otp, 0, drupal_strlen($otp) - 32);
}

function yubikey_validate($otp, &$errstr) {

  if (!variable_get('yubikey_enabled', FALSE)) {
    $errstr = "YubiKey Authentication Not Enabled";
    return FALSE;
  }

  $api_key = variable_get('yubikey_apikey', NULL);
  $id      = variable_get('yubikey_apiid', NULL);
  $timeout = variable_get('yubikey_timeout', 10);

  $api_url = "https://api.yubico.com/wsapi/verify";
  $sig_key = base64_decode($api_key);

  $hash = urlencode(base64_encode(hash_hmac("sha1", "id=$id&otp=$otp", $sig_key, TRUE)));

  $url  = "$api_url?id=$id&otp=$otp&h=$hash";

  $ch = curl_init($url);
  curl_setopt($ch, CURLOPT_TIMEOUT,        $timeout);
  curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, $timeout);
  curl_setopt($ch, CURLOPT_FOLLOWLOCATION, FALSE);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
  curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
  curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, TRUE);

  $result = explode("\n", curl_exec($ch));
  $error = curl_errno($ch);
  curl_close($ch);
  if ($error) {
    $errstr = "ERROR CONNECTING TO YUBICO - ". $error;
    return FALSE;
  }

  foreach ($result as $param) {
    if (drupal_substr($param, 0, 2) == "h=") {
      $signature = drupal_substr(trim($param), 2);
    }
    else if (drupal_substr($param, 0, 2) == "t=") {
      $timestamp = drupal_substr(trim($param), 2);
    }
    else if (drupal_substr($param, 0, 7) == "status=") {
      $status = drupal_substr(trim($param), 7);
    }
  }

  $signed = "status=". $status ."&t=". $timestamp;

  if (!(base64_encode(hash_hmac("sha1", $signed, $sig_key, TRUE)) == $signature)) {
    $errstr = 'BAD RESPONSE SIGNATURE';
    return FALSE;
  }

  if (!yubico_validate_timestamp($timestamp)) {
    $errstr = 'BAD TIMESTAMP';
    return FALSE;
  }

  if ($status != "OK") {
    $errstr = $status;
    return FALSE;
  }

  return TRUE;
}

function yubico_validate_timestamp($timestamp) {
  $tolerance = variable_get('yubikey_tolerance', 150);
  $now = date("U");
  $timestamp_seconds = date_format(date_create(drupal_substr($timestamp, 0, -4)), "U");

  if (!$timestamp || !$now) {
    return FALSE;
  }

  if ((($timestamp_seconds + $tolerance) > $now) && ($timestamp_seconds - $tolerance) < $now) {
    return TRUE;
  }

  return FALSE;
}
