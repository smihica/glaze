(def %qq-expand (x)
  (if (caris x 'unquote)
       (cadr x)

      (caris x 'unquote-splicing)
       (err "Illegal")

      (caris x 'quasiquote)
       (%qq-expand
         (%qq-expand (cadr x)))

      (acons x)
       (list 'join
             (%qq-expand-list (car x))
             (%qq-expand (cdr x)))

      (list 'quote x)))

(def %qq-expand-list (x)
  (if (caris x 'unquote)
      (list 'list (cadr x))

      (caris x 'unquote-splicing)
      (cadr x)

      (caris x 'quasiquote)
      (%qq-expand-list (%qq-expand (cadr x)))

      (acons x)
      (list 'list
            (list 'join
                  (%qq-expand-list (car x))
                  (%qq-expand (cdr x))))

      (list 'quote (list x))))

(mac quasiquote (x)
  (%qq-expand x))
