#!/usr/bin/guile -s
!#

(use-modules (srfi srfi-1)
	     (srfi srfi-8)
	     (srfi srfi-13))
(use-modules (ice-9 format)
	     (ice-9 ftw)
	     (ice-9 rdelim))

(define +BLD+ ".bld")
(define +BIN+ ".bin")
(define +LIB+ ".lib")

;; Configuration is very very simple.
;; Any pairs of CPP + HPP are considered to be objects and will be included in
;; the static library, which goes in the lib dir. Any lone CPP files will be
;; compiled into binaries and put in the bin dir. Lone HPP files are ignored.

(define (is-dir? filename)
  (eq? 'directory (stat:type (stat filename))))

(define (ls-sources)
  (filter-map (lambda (filename)
		(and (not (is-dir? filename))
		     (< 4 (string-length filename))
		     (equal? ".cpp" (string-take-right filename 4))
		     (first (string-split filename #\.))))
	      (scandir ".")))

(define (sort-sources)
  (partition (lambda (name)
	       (file-exists? (string-append name ".hpp")))
	     (ls-sources)))

(define (project-name)
  (last (string-split (getcwd) #\/)))

;; directory staging
(for-each (lambda (dirname)
	    (unless (and (file-exists? dirname)
			 (is-dir? dirname))
	      (mkdir dirname)))
	  (list +BLD+ +BIN+ +LIB+))

(with-output-to-file "Makefile"
  (lambda ()
    (with-input-from-file "base.mk"
      (lambda ()
	(let loop ((line (read-line)))
	  (unless (eof-object? line)
	    (display line) (newline)
	    (loop (read-line))))))

    (receive (obj-names bin-names) (sort-sources)
      (let ((lib-name (project-name)))
	;; write object file list
	(newline)
	(format #t "OBJS =")
	(for-each (lambda (obj-name)
		    (format #t " ~a/~a.o" +BLD+ obj-name))
		  obj-names)
	(format #t "~%")

	;; write all-target
	(newline)
	(format #t "all: ~a/lib~a.a" +LIB+ lib-name)
	(for-each (lambda (bin-name)
		    (format #t " ~a/~a" +BIN+ bin-name))
		  bin-names)
	(format #t "~%")

	;; write clean target
	(newline)
	(format #t "clean:~%")
	(format #t "~/rm ~a/* ~a/* ~a/*~%" +BIN+ +BLD+ +LIB+)

	;; write bin targets
	(for-each
	 (lambda (bin-name)
	   (newline)
	   (format #t "~a/~a: ~a.cpp ~a/lib~a.a~%"
		   +BIN+ bin-name bin-name +LIB+ lib-name)
	   (format #t "~/$(CC) -o ~a/~a $(CFG) ~a.cpp -L~a -l~a $(LFG)~%"
		   +BIN+ bin-name bin-name +LIB+ lib-name))
	 bin-names)

	;; write static lib target
	(newline)
	(format #t "~a/lib~a.a: $(OBJS)~%" +LIB+ lib-name)
	(format #t "~/ar rcs ~a/lib~a.a $(SFG) $(OBJS)~%" +LIB+ lib-name)

	;; write object code targets
	(for-each (lambda (obj-name)
		    (newline)
		    (format #t "~a/~a.o: ~a.cpp ~a.hpp~%"
			    +BLD+ obj-name obj-name obj-name)
		    (format #t "~/$(CC) -o ~a/~a.o -c $(CFG) ~a.cpp $(LFG)~%"
			    +BLD+ obj-name obj-name))
		  obj-names)))))
