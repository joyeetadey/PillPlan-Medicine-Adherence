# Generated by Django 3.2.6 on 2021-11-10 21:37

from django.db import migrations


class Migration(migrations.Migration):

    dependencies = [
        ('users', '0019_medicine'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='myuser',
            name='date_joined',
        ),
        migrations.RemoveField(
            model_name='myuser',
            name='emergency_email',
        ),
        migrations.RemoveField(
            model_name='myuser',
            name='emergency_name',
        ),
        migrations.RemoveField(
            model_name='myuser',
            name='filled_status',
        ),
        migrations.RemoveField(
            model_name='myuser',
            name='user_type',
        ),
    ]