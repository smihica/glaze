(def accumulate (op initial sequence)
  (if (no sequence)
      initial
      (op (car sequence)
          (accumulate op initial (cdr sequence)))))

(def append (seq1 seq2)
  (accumulate cons seq2 seq1))

(def map1 (f xs)
  (if xs
      (cons (f (car xs)) (map1 f (cdr xs)))))

(def cadr (x) (car (cdr x)))
(def cddr (x) (cdr (cdr x)))

(def pair (xs)
  (if (no xs)
       nil
      (no (cdr xs))
       (list (list (car xs)))
      (cons (list (car xs) (cadr xs))
            (pair (cddr xs)))))

(mac with (parms . body)
  (append (list (append (list 'fn (map1 car (pair parms)))
                        body))
          (map1 cadr (pair parms))))

(mac let (var val . body)
  (append (list 'with (list var val)) body))