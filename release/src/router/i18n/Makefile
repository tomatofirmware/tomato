clean:
	rm www/www.pot
	rm www/*.mo
pot:
	xgettext -d www -L PHP --package-name=TomatoEgg -k=_:1 ../www/*.asp ../www/tomato.js --from-code=utf-8 -o www/www.pot
mo:
	msgfmt www/en_US.po -o www/en_US.mo
	msgfmt www/zh_CN.po -o www/zh_CN.mo
merge:
	msgmerge --update --no-fuzzy-matching --backup=off www/en_US.po www/www.pot
	msgmerge --update --no-fuzzy-matching --backup=off www/zh_CN.po www/www.pot
