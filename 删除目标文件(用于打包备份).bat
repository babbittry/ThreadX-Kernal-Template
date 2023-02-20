rd /Q /S project\EmbeddedStudio\Output

rd /Q /S project\MDK-ARM(AC5)\Objects
rd /Q /S project\MDK-ARM(AC5)\Listings
rd /Q /S project\MDK-ARM(AC5)\DebugConfig
del /Q project\MDK-ARM(AC5)\*.bak
del /Q project\MDK-ARM(AC5)\*.dep
del /Q project\MDK-ARM(AC5)\JLink*
del /Q project\MDK-ARM(AC5)\project.uvgui.*

rd /Q /S project\MDK-ARM(AC6)\Objects
rd /Q /S project\MDK-ARM(AC6)\Listings
rd /Q /S project\MDK-ARM(AC6)\DebugConfig
del /Q project\MDK-ARM(AC6)\*.bak
del /Q project\MDK-ARM(AC6)\*.dep
del /Q project\MDK-ARM(AC6)\JLink*
del /Q project\MDK-ARM(AC6)\project.uvgui.*


del /Q project\EWARMv8\Project.dep
del /Q project\EWARMv8\Debug
del /Q project\EWARMv8\Flash
del /Q project\EWARMv8\settings
del /Q project\EWARMv8\Debug
rd  /Q /S project\EWARMv8\Flash
rd /Q /S project\EWARMv8\settings
rd /Q /S project\EWARMv8\Debug

del *.bak /s
del *.ddk /s
del *.edk /s
del *.lst /s
del *.lnp /s
del *.mpf /s
del *.mpj /s
del *.obj /s
del *.omf /s
::del *.opt /s  ::不允许删除JLINK的设置
del *.plg /s
del *.rpt /s
del *.tmp /s
del *.__i /s
del *.crf /s
del *.o /s
del *.d /s
del *.axf /s
del *.tra /s
del *.dep /s           
del JLinkLog.txt /s

del *.iex /s
del *.htm /s
del *.sct /s
del *.map /s

del *.dbgconf /s
del *.LINGZHUNING /s
del *.Administrator /s
del *.Mr_Xu /s
exit

