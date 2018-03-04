import os
class BuilderBase:
  rootPath = os.path.abspath(os.path.join(__file__,os.pardir,os.pardir,os.path.pardir))
  default_cfg_all = {
  "appName": "LGML",
  "njobs" : 1,
  "lgml_root_path" : rootPath,
  "build_cfg_name" : "Debug",

  }

  def __init__(self,cfg=None):
    self.cfg = cfg or {};
    self.verbose = "normal" # or quiet or verbose
    self.applyCfg(self.default_cfg_all)


  def fillVersionInfo(self):
    targetContent = """
// this file is autogenerated in deploy builds to set full build name
// BUILD_VERSION_UID is only used in auto updater
#define BUILD_VERSION_UID \"%s\"
"""%self.getShortUID()
    with open(os.path.join(self.rootPath,"Source","Version.h"),'r') as f:
      content = f.read();
      needUpdate = (content!=targetContent)
    if needUpdate:
      with open(os.path.join(self.rootPath,"Source","Version.h"),'w') as f:
        f.write(targetContent);

  def applyCfg(self, newCfg,overwrite = False):
    for k in newCfg:
      if overwrite or not k in self.cfg or self.cfg[k] is None:
        self.cfg[k] = newCfg[k]

  def getNameWithVersion(self):
    from PyUtils import ProJucerUtils
    name =  self.cfg["appName"]+ "_v"+str(ProJucerUtils.getVersion())
    return name
  

  def getBinaryPath(self):
    if "binary_path" in self.cfg:
      return self.cfg["binary_path"]
    else :
      raise NameError("no binary_path provided")

  def getPreprocessor(self):
    return ""

  def getPlatformName(self):
    raise NotImplementedError("should be implemented by builder")
  
  def getUID(self):
    return "%s_%s_%s"%(self.getNameWithVersion(),self.getShortUID(),self.cfg["build_cfg_name"])

  def getShortUID(self):
    suffix = self.cfg["packagesuffix"]
    if suffix is not None:
      return "%s_%s_%s"%(self.getPlatformName(),self.cfg["arch"],suffix)
    else:
      return "%s_%s"%(self.getPlatformName(),self.cfg["arch"])


  def getReadmePath(self):
    return os.path.join(self.rootPath,"README.md");

  # default package method zips application file
  def packageApp(self,export_path):
    return self.zipApp(export_path)

  def zipApp(self,export_path):
    import zipfile
    source =self.getBinaryPath();
    if not os.path.exists(export_path):
      raise NameError("can't package to %s"%export_path)
    if not os.path.exists(source):
      raise NameError("nothing to package (%s)"%source)

    zipname = self.getUID()+".zip"
    zipPath = os.path.join(export_path,zipname)
    if(os.path.isfile(source)):
      with zipfile.ZipFile(zipPath,'w') as z:
        z.write(source,arcname=os.path.basename(source))
    else:
      zipf = zipfile.ZipFile(zipPath, 'w', zipfile.ZIP_DEFLATED)
      zipdir(source, zipf,os.path.abspath(os.path.join(source,os.path.pardir)))
      zipf.close()

    return zipPath




def zipdir(path, ziph,baseDir):
    # ziph is zipfile handle
    lenBase = len(baseDir)
    for root, dirs, files in os.walk(path):
        for file in files:
            fullp = os.path.join(root, file)
            print ("zipping",fullp[lenBase:])
            ziph.write(fullp,arcname=fullp[lenBase:])
        for d in dirs:
          zipdir(d,ziph,baseDir)

