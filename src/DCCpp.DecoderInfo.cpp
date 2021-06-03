/*************************************************************
  project: <DCCpp library>
  author: <Thierry PARIS>
  description: <DCCpp class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

/*
* Rev 6 March 2021
* https://www.nmra.org/sites/default/files/standards/sandrp/pdf/appendix_a_s-9_2_2_5.pdf
*/

DecoderManufacturer DCCpp::DecoderManufacturers[] =
{
  /******** 000 *********/
  { "", 0, "" },
  {"CML Electronics Limited", 1, "UK", "CML_Systems" },
  {"Train Technology", 2, "BE" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },

  /******** 010 *********/
  { "", 0, "" },
  {"NCE Corporation(formerly North Coast Engineering)", 11, "US", "NCE" },
  {"Wangrow Electronics", 12, "US", "Wangrow" },
  {"Public Domain & Do - It - Yourself Decoders", 13, " -" },
  {"PSI –Dynatrol/Lathi Associates", 14, "US" },
  {"Ramfixx Technologies(Wangrow)", 15, "CA / US" },
  { "", 0, "" },
  {"Advance IC Engineering", 17, "US" },
  {"JMRI", 18, "US" },
  {"AMW", 19, "AT" },

  /******* 020 *******/
  {"T4T – Technology for Trains GmbH", 20, "DE" },
  {"Kreischer Datentechnik", 21, "DE" },
  {"KAM Industries", 22, "US" },
  {"S Helper Service", 23, "US" },
  {"MoBaTron.de", 24, "DE" },
  {"Team Digital, LLC", 25, "US", "TD" },
  {"MBTronik – PiN GITmBH", 26, "DE" },
  {"MTH Electric Trains, Inc.", 27, "US", "MTH" },
  {"Heljan A / S", 28, "DK" },
  {"Mistral Train Models", 29, "BE", "Mistral" },

  /******** 030 ********/
  {"Digsight", 30, "CN" },
  {"Brelec", 31, "BE" },
  {"Regal Way Co.Ltd", 32, "HKG" },
  {"Praecipuus", 33, "CA" },
  {"Aristo - Craft Trains", 34, "US" },
  {"Electronik & Model Produktion", 35, "SE" },
  {"DCCconcepts", 36, "AU", "DCC_Concepts" },
  {"NAC Services, Inc", 37, "US" },
  {"Broadway Limited Imports, LLC", 38, "US", "BLI" },
  {"Educational Computer, Inc.", 39, "US" },

  /********* 040 *********/
  {"KATO Precision Models", 40, "JP", "Kato" },
  {"Passmann", 41, "DE" },
  {"Digikeijs", 42, "NL", "Digikeijs/Digirails" },
  {"Ngineering", 43, "US" },
  {"SPROG - DCC", 44, "UK", "SPROG_DCC" },
  {"ANE Model Co, Ltd", 45, "TWN", "ANE" },
  {"GFB Designs", 46, "UK", "GFB" },
  {"Capecom", 47, "AU" },
  {"Hornby Hobbies Ltd", 48, "UK", "Hornby" },
  {"Joka Electronic", 49, "DE" },

  /********* 050 *********/
  {"N & Q Electronics", 50, "ESP" },
  {"DCC Supplies, Ltd", 51, "UK" },
  {"Krois - Modell", 52, "AT" },
  {"Rautenhaus Digital Vertrieb", 53, "DE", "Rautenhaus" },
  {"TCH Technology", 54, "US" },
  {"QElectronics GmbH", 55, "DE", "Qdecoder" },
  {"LDH", 56, "ARG", "LDH" },
  {"Rampino Elektronik", 57, "DE" },
  {"KRES GmbH", 58, "DE" },
  {"Tam Valley Depot", 59, "US", "TamValleyDepot" },

  /********* 060 *********/
  {"Bluecher - Electronic", 60, "DE" },
  {"TrainModules", 61, "HUN" },
  {"Tams Elektronik GmbH", 62, "DE", "TAMS" },
  {"Noarail", 63, "AUS" },
  {"Digital Bahn", 64, "DE" },
  {"Gaugemaster", 65, "UK", "Gaugemaster" },
  {"Railnet Solutions, LLC", 66, "US" },
  {"Heller Modenlbahn", 67, "DE" },
  {"MAWE Elektronik", 68, "CH" },
  {"E - Modell", 69, "DE" },

  /********* 070 *********/
  {"Rocrail", 70, "DE" },
  {"New York Byano Limited", 71, "HK", "NYB" },
  {"MTB Model", 72, "CZE" },
  {"The Electric Railroad Company", 73, "US" },
  {"PpP Digital", 74, "ESP", "PpP" },
  {"Digitools Elektronika, Kft", 75, "HUN" },
  {"Auvidel", 76, "DE" },
  {"LS Models Sprl", 77, "BEL" },
  {"Tehnologistic(train - O - matic)", 78, "ROM", "TrainOMatic" },
  {"Hattons Model Railways", 79, "UK", "Hattons" },

  /********* 080 *********/
  {"Spectrum Engineering", 80, "US", "Spectrum" },
  {"GooVerModels", 81, "BEL" },
  {"HAG Modelleisenbahn AG", 82, "CHE" },
  {"JSS - Elektronic", 83, "DE" },
  {"Railflyer Model Prototypes, Inc.", 84, "CAN" },
  {"Uhlenbrock GmbH", 85, "DE", "Uhlenbrock" },
  {"Wekomm Engineering, GmbH", 86, "DE", "Wekomm" },
  {"RR - Cirkits", 87, "US", "RR-Cirkits" },
  {"HONS Model", 88, "HKG" },
  {"Pojezdy.EU", 89, "CZE" },

  /********* 090 *********/
  {"Shourt Line", 90, "US" },
  {"Railstars Limited", 91, "US" },
  {"Tawcrafts", 92, "UK" },
  {"Kevtronics cc", 93, "ZAF" },
  {"Electroniscript, Inc.", 94, "US", "SDD" },
  {"Sanda Kan Industrial, Ltd.", 95, "HKG" },
  {"PRICOM Design", 96, "US" },
  {"Doehler & Haas", 97, "DE", "Doehler_Haas" },
  {"Harman DCC", 98, "UK", "Harman" },
  {"Lenz Elektronik GmbH", 99, "DE", "Lenz" },

  /********* 100 *********/
  {"Trenes Digitales", 100, "ARG" },
  {"Bachmann Trains", 101, "US", "Bachmann" },
  {"Integrated Signal Systems", 102, "US" },
  {"Nagasue System Design Office", 103, "JP" },
  {"TrainTech", 104, "NL" },
  {"Computer Dialysis France", 105, "FR" },
  {"Opherline1", 106, "FR" },
  {"Phoenix Sound Systems, Inc.", 107, "US" },
  {"Nagoden", 108, "JP" },
  {"Viessmann Modellspielwaren GmbH", 109, "DE", "Viessmann" },

  /********* 110 *********/
  { "AXJ Electronics", 110, "CHN" },
  {"Haber & Koenig Electronics GmbH(HKE)", 111, "AT", "Haber_u_Koenig" },
  {"LSdigital", 112, "DE" },
  {"QS Industries(QSI)", 113, "US", "QSI" },
  {"Benezan Electronics", 114, "ESP" },
  {"Dietz Modellbahntechnik", 115, "DE" },
  {"MyLocoSound", 116, "AUS", "MyLocoSound" },
  {"cT Elektronik", 117, "AT", "CT" },
  {"MÜT GmbH", 118, "DE" },
  {"W.S.Ataras Engineering", 119, "US" },

  /********* 120 *********/
  {"csikos - muhely", 120, "HUN" },
  {"ECCO GmbH", 121, "DE" },
  {"Berros", 122, "NL" },
  {"Massoth Elektronik, GmbH", 123, "DE", "Massoth" },
  {"DCC - Gaspar - Electronic", 124, "HUN" },
  {"ProfiLok Modellbahntechnik GmbH", 125, "DE" },
  {"Möllehem Gårdsproduktion", 126, "SE", "MGP" },
  {"Atlas Model Railroad Products", 127, "US", "Atlas" },
  {"Frateschi Model Trains", 128, "BRA" },
  {"Digitrax", 129, "US", "Digitrax" },

  /********* 130 *********/
  {"cmOS Engineering", 130, "AUS" },
  {"Trix Modelleisenbahn", 131, "DE", "Trix" },
  {"ZTC", 132, "UK", "ZTC" },
  {"Intelligent Command Control", 133, "US" },
  {"LaisDCC", 134, "CHN", "LaisDcc" },
  {"CVP Products", 135, "US", "CVProducts" },
  {"NYRS", 136, "US" },
  { "A-Train Electronics", 137, "" },
  { "Train ID Systems", 138, "US" },
  { "RealRail Effects", 139, "US" },

  /********* 140 *********/
  {"Desktop Station", 140, "JP" },
  {"Throttle - Up(Soundtraxx)", 141, "US", "SoundTraxx" },
  {"SLOMO Railroad Models", 142, "JP" },
  {"Model Rectifier Corp.", 143, "US", "MRC" },
  {"DCC Train Automation", 144, "UK" },
  {"Zimo Elektronik", 145, "AT", "Zimo" },
  {"Rails Europ Express", 146, "FR" },
  {"Umelec Ing.Buero", 147, "CH", "Umelec" },
  {"BLOCKsignalling", 148, "UK" },
  {"Rock Junction Controls", 149, "US" },

  /********* 150 *********/
  {"Wm.K.Walthers, Inc.", 150, "US" },
  {"Electronic Solutions Ulm GmbH", 151, "DE", "ESU" },
  {"Digi - CZ", 152, "CZE" },
  {"Train Control Systems", 153, "US", "TCS" },
  {"Dapol Limited", 154, "UK" },
  {"Gebr.Fleischmann GmbH & Co.", 155, "DE", "Fleischmann/Fl" },
  {"Nucky", 156, "JP" },
  {"Kuehn Ing.", 157, "DE", "Kuehn" },
  {"Fucik", 158, "CZE", "Fucik/Public_Domain_Fucik" },
  {"LGB(Ernst Paul Lehmann Patentwerk)", 159, "DE" },

  /********* 160 *********/
  {"MD Electronics", 160, "DE" },
  {"Modelleisenbahn GmbH(formerly Roco)", 161, "AT" },
  {"PIKO", 162, "DE" },
  {"WP Railshops", 163, "CA" },
  {"drM", 164, "TWN" },
  {"Model Electronic Railway Group", 165, "UK", "MERG/Public_Domain_MERG" },
  {"Maison de DCC", 166, "JP" },
  {"Helvest Systems GmbH", 167, "CH" },
  {"Model Train Technology", 168, "US" },
  {"AE Electronic Ltd.", 169, "HN" },

  /********* 170 *********/
  {"AuroTrains ", 170, "US / IT" },
  { "", 0, "" },
  { "", 0, "" },
  {"Arnold – Rivarossi", 173, "DE", "Arnold" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },

  /********* 180 *********/
  { "", 180, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  {"BRAWA Modellspielwaren GmbH & Co.", 186, "DE" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },

  /********* 190 *********/
  { "", 190, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },

  /********* 200 *********/
  { "", 200, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  {"Con - Com GmbH", 204, "AT" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },

  /********* 210 *********/
  { "", 210, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },

  /********* 220 *********/
  { "", 220, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  {"Blue Digital", 225, "POL" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },

  /********* 230 *********/
  { "", 230, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  {"NMRA Reserved(for extended ID #’s)", 238, "US" },
  { "", 0, "" },

  /********* 240 *********/
  { "", 240, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },

  /********* 250 *********/
  { "", 250, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" },
  { "", 0, "" }
};

DecoderModel decoderModel;

DecoderModel *DCCpp::getDecoderInfo(volatile RegisterList* inpRegs)
{
  int cv8 = DCCpp::readCv(inpRegs, 8, 100, 200);

  if (cv8 == -1)
    return NULL;

  int cv7 = DCCpp::readCv(inpRegs, 7, 100, 200);

  decoderModel.ManufacturerCv8 = cv8;
  decoderModel.Cv7 = cv7;

  return &decoderModel;
}

int DCCpp::identifyLocoId(volatile RegisterList* inpRegs)
{
  int cv29;
  cv29 = DCCpp::readCv(inpRegs, 29, 100, 200);
  if (cv29 == -1)
    return -1;

  int id = -1;
  if (bitRead(cv29, 5))
  {
    // long address : get CV#17 and CV#18
    int cv18 = DCCpp::readCv(inpRegs, 18, 100, 200);
    if (cv18 != -1)
    {
      int cv17 = DCCpp::readCv(inpRegs, 17, 100, 200);
      if (cv17 != -1)
      {
        id = cv18 + ((cv17 - 192) << 8);
      }
    }
  }
  else
  {
    // short address: read only CV#1
    id = DCCpp::readCv(inpRegs, 1, 100, 200);
  }

  return (id);
}
