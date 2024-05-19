{ pkgs, config, lib, ... }:
let
  cfg = config.TLMS.r09-receiver;
in
{
  options.TLMS.r09-receiver = with lib; {
    enable = mkOption {
      type = types.bool;
      default = false;
      description = ''Wether to enable TLMS r09-receiver'';
    };
    device = mkOption {
      type = types.str;
      default = "";
      example = "hackrf=0";
      description = ''Device string to pass to r09-receiver'';
    };
    frequency = mkOption {
      type = types.int;
      default = 170795000;
      description = ''Frequency to tune radio to'';
    };
    offset = mkOption {
      type = types.int;
      default = 19550;
      description = ''Offset of the signal from center frequency'';
    };
    RF = mkOption {
      type = types.int;
      default = 14;
      description = "";
    };
    IF = mkOption {
      type = types.int;
      default = 32;
      description = "";
    };
    BB = mkOption {
      type = types.int;
      default = 42;
      description = "";
    };
    user = mkOption {
      type = types.str;
      default = "r09-receiver";
      description = "as which user r09-receiver should run";
    };
    group = mkOption {
      type = types.str;
      default = "r09-receiver";
      description = "as which group r09-receiver should run";
    };
  };

  config = lib.mkIf cfg.enable {

    hardware = {
      hackrf.enable = true;
      rtl-sdr.enable = true;
    };

    environment.systemPackages = [ pkgs.r09-receiver ];

    systemd.services."r09-receiver" = {
      enable = true;
      wantedBy = [ "multi-user.target" ];

      script = "exec ${pkgs.r09-receiver}/bin/r09-receiver &";

      environment = with cfg; {
        "DECODER_FREQUENCY" = toString frequency;
        "DECODER_OFFSET" = toString offset;
        "DECODER_RF" = toString RF;
        "DECODER_IF" = toString IF;
        "DECODER_BB" = toString BB;
        "DECODER_DEVICE_STRING" = device;
      };

      serviceConfig = {
        Type = "forking";
        User = cfg.user;
        Restart = "on-failure";
        StartLimitBurst = "2";
        StartLimitIntervalSec = "150s";
      };
    };

    users.groups."${cfg.group}" = { };
    users.users."${cfg.user}" = {
      name = cfg.user;
      description = "r09-receiver service user";
      isNormalUser = true;
      group = cfg.group;
      extraGroups = [ "plugdev" ];
    };

    security.wrappers.r09-receiver = {
      owner = cfg.user;
      group = "users";
      capabilities = "cap_sys_nice+eip";
      source = "${pkgs.r09-receiver}/bin/r09-receiver";
    };

  };
}

